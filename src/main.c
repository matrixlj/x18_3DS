#include <citro2d.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

// ============================================================================
// SCREEN DEFINITIONS
// ============================================================================

#define SCREEN_WIDTH_TOP    400
#define SCREEN_HEIGHT_TOP   240
#define SCREEN_WIDTH_BOT    320
#define SCREEN_HEIGHT_BOT   240

// ============================================================================
// MIXER DEFINITIONS
// ============================================================================

#define NUM_FADERS 16
#define FADERS_PER_ROW 16
#define FADER_WIDTH (SCREEN_WIDTH_BOT / FADERS_PER_ROW)
#define FADER_HEIGHT SCREEN_HEIGHT_BOT
#define FADER_BAR_WIDTH 3
#define BUTTON_HEIGHT 14

// ============================================================================
// SHOW MANAGER DEFINITIONS
// ============================================================================

#define SHOWS_DIR "/3ds/x18mixer/shows/"
#define MAX_SHOWS 64
#define APP_MODE_MIXER 0
#define APP_MODE_MANAGER 1

// Manager buttons
#define BTN_LOAD 0
#define BTN_DELETE 1
#define BTN_DUPLICATE 2
#define BTN_RENAME 3
#define NUM_BUTTONS 4

typedef struct {
    int id;
    float value;
    int muted;
    int eq_enabled;
    float x, y;
    float w, h;
} Fader;

// ============================================================================
// PRESET/SHOW DEFINITIONS
// ============================================================================

typedef struct {
    char name[32];
    float volumes[16];
    int mutes[16];
    int eqs[16];
} Step;

typedef struct {
    char name[64];
    Step steps[200];
    int num_steps;
} Show;

// ============================================================================
// GLOBAL STATE
// ============================================================================

// Screens
typedef struct {
    C3D_RenderTarget* target;
    int width;
    int height;
} Screen;

Screen g_topScreen, g_botScreen;
C2D_TextBuf g_textBuf = NULL;

// Fader sprite sheets and images
C2D_SpriteSheet g_grip_sheet = NULL;     // Grip sprite sheet
C2D_Image g_grip_img = {0};              // Grip/slider image
C2D_SpriteSheet g_fader_sheet = NULL;    // Fader background sprite sheet
C2D_Image g_fader_bkg = {0};             // Fader background image

// Image loading status for debugging
int g_romfs_mounted = 0;
int g_grip_loaded = 0;
int g_fader_loaded = 0;

// Mixer
Fader g_faders[NUM_FADERS];

// Touch input
touchPosition g_touchPos = {0, 0};
int g_isTouched = 0;
int g_wasTouched = 0;

// Show/Preset system
Show g_current_show;
int g_show_loaded = 0;
int g_selected_step = 0;

// Show Manager state
int g_app_mode = APP_MODE_MIXER;
char g_available_shows[MAX_SHOWS][64];
int g_num_available_shows = 0;
int g_selected_show = 0;
char g_new_name[64] = "";
int g_renaming = 0;
int g_rename_input_pos = 0;

// New show naming state
int g_creating_new_show = 0;
int g_new_show_input_pos = 0;
char g_new_show_name[64] = "";

// Modification tracking
int g_show_modified = 0;  // 1 if show has unsaved changes

// Exit flag
int g_should_exit = 0;  // Set to 1 to signal app should close

// Debug/Status
char g_save_status[128] = "Ready";
int g_save_status_timer = 0;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

int load_show_from_file(const char *filename, Show *out_show);
void apply_step_to_faders(int step_idx);
void save_show_to_file(Show *show);
void add_step(void);
void duplicate_step(void);

// ============================================================================
// SHOW/PRESET FUNCTIONS
// ============================================================================

void init_default_show(void)
{
    // Try to load last saved show from persistence
    FILE *f = fopen("/3ds/x18mixer/last_show.txt", "r");
    if (f) {
        char last_show[64] = "";
        if (fgets(last_show, sizeof(last_show), f)) {
            // Remove newline if present
            char *nl = strchr(last_show, '\n');
            if (nl) *nl = '\0';
            
            if (load_show_from_file(last_show, &g_current_show)) {
                fclose(f);
                g_show_loaded = 1;
                g_selected_step = 0;
                g_show_modified = 0;  // Loaded from disk, not modified
                apply_step_to_faders(0);
                return;
            }
        }
        fclose(f);
    }
    
    // If no saved show found, create default
    strcpy(g_current_show.name, "Default Show");
    g_current_show.num_steps = 3;
    
    for (int s = 0; s < 3; s++) {
        snprintf(g_current_show.steps[s].name, sizeof(g_current_show.steps[s].name), "Step %d", s + 1);
        
        for (int i = 0; i < 16; i++) {
            g_current_show.steps[s].volumes[i] = 0.5f;
            g_current_show.steps[s].mutes[i] = 0;
            g_current_show.steps[s].eqs[i] = 0;
        }
    }
    g_show_loaded = 1;
    g_show_modified = 0;  // Default show is not modified
}

void init_new_show(const char *name)
{
    // Initialize a brand new show with default 3 steps
    strcpy(g_current_show.name, name);
    g_current_show.num_steps = 3;
    
    for (int s = 0; s < 3; s++) {
        snprintf(g_current_show.steps[s].name, sizeof(g_current_show.steps[s].name), "Step %d", s + 1);
        
        for (int i = 0; i < 16; i++) {
            g_current_show.steps[s].volumes[i] = 0.5f;
            g_current_show.steps[s].mutes[i] = 0;
            g_current_show.steps[s].eqs[i] = 0;
        }
    }
    g_show_loaded = 1;
    g_show_modified = 1;  // New show is modified (not yet saved)
    g_selected_step = 0;
}

void apply_step_to_faders(int step_idx)
{
    if (step_idx < 0 || step_idx >= g_current_show.num_steps) return;
    
    Step *step = &g_current_show.steps[step_idx];
    for (int i = 0; i < 16; i++) {
        g_faders[i].value = step->volumes[i];
        g_faders[i].muted = step->mutes[i];
        g_faders[i].eq_enabled = step->eqs[i];
    }
    g_selected_step = step_idx;
}

void save_step_from_faders(int step_idx)
{
    if (step_idx < 0 || step_idx >= g_current_show.num_steps) return;
    
    Step *step = &g_current_show.steps[step_idx];
    for (int i = 0; i < 16; i++) {
        step->volumes[i] = g_faders[i].value;
        step->mutes[i] = g_faders[i].muted;
        step->eqs[i] = g_faders[i].eq_enabled;
    }
}

void add_step(void)
{
    // Cannot add more than 200 steps
    if (g_current_show.num_steps >= 200) {
        snprintf(g_save_status, sizeof(g_save_status), "Cannot add more than 200 steps");
        g_save_status_timer = 120;
        return;
    }
    
    int new_idx = g_current_show.num_steps;
    Step *new_step = &g_current_show.steps[new_idx];
    
    // Create new step with default values
    snprintf(new_step->name, sizeof(new_step->name), "Step %d", new_idx + 1);
    
    for (int i = 0; i < 16; i++) {
        new_step->volumes[i] = 0.5f;
        new_step->mutes[i] = 0;
        new_step->eqs[i] = 0;
    }
    
    g_current_show.num_steps++;
    g_selected_step = new_idx;
    g_show_modified = 1;  // Mark as modified
    apply_step_to_faders(new_idx);
    
    snprintf(g_save_status, sizeof(g_save_status), "Added Step %d", new_idx + 1);
    g_save_status_timer = 120;
}

void duplicate_step(void)
{
    // Cannot add more than 200 steps
    if (g_current_show.num_steps >= 200) {
        snprintf(g_save_status, sizeof(g_save_status), "Cannot add more than 200 steps");
        g_save_status_timer = 120;
        return;
    }
    
    // Get current step
    if (g_selected_step < 0 || g_selected_step >= g_current_show.num_steps) {
        snprintf(g_save_status, sizeof(g_save_status), "Invalid step to duplicate");
        g_save_status_timer = 120;
        return;
    }
    
    Step *src_step = &g_current_show.steps[g_selected_step];
    int new_idx = g_current_show.num_steps;
    Step *new_step = &g_current_show.steps[new_idx];
    
    // Copy the step
    memcpy(new_step, src_step, sizeof(Step));
    
    // Build name safely using temp buffer
    char temp_name[64];
    snprintf(temp_name, sizeof(temp_name), "%s copy", src_step->name);
    strcpy(new_step->name, temp_name);
    
    g_current_show.num_steps++;
    g_selected_step = new_idx;
    g_show_modified = 1;  // Mark as modified
    apply_step_to_faders(new_idx);
    
    snprintf(g_save_status, sizeof(g_save_status), "Duplicated Step %d", new_idx + 1);
    g_save_status_timer = 120;
}

// ============================================================================
// SHOW MANAGER FILE I/O
// ============================================================================

void create_shows_directory(void)
{
    // Create /3ds/ if it doesn't exist
    mkdir("/3ds", 0755);
    // Create /3ds/x18mixer/ if it doesn't exist
    mkdir("/3ds/x18mixer", 0755);
    // Create /3ds/x18mixer/shows/ if it doesn't exist
    mkdir(SHOWS_DIR, 0755);
}

// Test filesystem write capability
int test_filesystem_write(void)
{
    create_shows_directory();
    
    char test_path[256];
    snprintf(test_path, sizeof(test_path), "%stest.txt", SHOWS_DIR);
    
    FILE *f = fopen(test_path, "wb");
    if (!f) {
        snprintf(g_save_status, sizeof(g_save_status), "ERROR: Cannot open file at %s", SHOWS_DIR);
        return 0;
    }
    
    // Try to write test data
    const char *test_data = "TEST";
    size_t written = fwrite(test_data, 1, 4, f);
    fflush(f);
    fsync(fileno(f));
    fclose(f);
    
    if (written != 4) {
        snprintf(g_save_status, sizeof(g_save_status), "ERROR: Write failed (wrote %zu bytes)", written);
        return 0;
    }
    
    snprintf(g_save_status, sizeof(g_save_status), "OK: Filesystem test passed! Saved to %s", SHOWS_DIR);
    return 1;
}

// Sanitize filename - replace spaces and invalid chars with underscores
void sanitize_filename(const char *input, char *output, int max_len)
{
    if (!input || !output) return;
    
    int i = 0;
    for (i = 0; i < max_len - 1 && input[i] != '\0'; i++) {
        char c = input[i];
        // Allow alphanumerics, underscore, hyphen, dot
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || 
            (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.') {
            output[i] = c;
        } else {
            // Replace spaces and other chars with underscore
            output[i] = '_';
        }
    }
    output[i] = '\0';
}

void save_show_to_file(Show *show)
{
    if (!show) return;
    
    create_shows_directory();
    
    // Sanitize the show name for use as a filename
    char safe_name[64];
    sanitize_filename(show->name, safe_name, sizeof(safe_name));
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s%s.x18s", SHOWS_DIR, safe_name);
    
    // Try to open file for writing
    FILE *f = fopen(filepath, "wb");
    if (!f) {
        snprintf(g_save_status, sizeof(g_save_status), "ERROR: Cannot save file");
        g_save_status_timer = 120;  // Show for 2 seconds
        return;
    }
    
    // Write the show struct
    size_t written = fwrite(show, sizeof(Show), 1, f);
    
    // Ensure data is written
    fflush(f);
    fsync(fileno(f));
    fclose(f);
    
    // Report status
    if (written == 1) {
        snprintf(g_save_status, sizeof(g_save_status), "SAVED: %s (%d steps)", safe_name, show->num_steps);
        g_save_status_timer = 120;  // Show for 2 seconds
        
        // Update last show persistence file
        FILE *pf = fopen("/3ds/x18mixer/last_show.txt", "w");
        if (pf) {
            fprintf(pf, "%s", safe_name);
            fflush(pf);
            fsync(fileno(pf));
            fclose(pf);
        }
        
        g_show_modified = 0;  // Mark as saved
    } else {
        snprintf(g_save_status, sizeof(g_save_status), "ERROR: Write failed for %s", safe_name);
        g_save_status_timer = 120;
    }
}

int load_show_from_file(const char *filename, Show *out_show)
{
    if (!filename || !out_show) return 0;
    
    create_shows_directory();
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s%s.x18s", SHOWS_DIR, filename);
    
    FILE *f = fopen(filepath, "rb");
    if (!f) return 0;
    
    // Read the show struct
    size_t read = fread(out_show, sizeof(Show), 1, f);
    fclose(f);
    
    // Ensure we read the complete structure
    return (read == 1) ? 1 : 0;
}

void list_available_shows(void)
{
    g_num_available_shows = 0;
    create_shows_directory();
    
    DIR *dir = opendir(SHOWS_DIR);
    if (!dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) && g_num_available_shows < MAX_SHOWS) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".x18s")) {
            // Remove .x18s extension
            strcpy(g_available_shows[g_num_available_shows], entry->d_name);
            char *dot = strstr(g_available_shows[g_num_available_shows], ".x18s");
            if (dot) *dot = '\0';
            g_num_available_shows++;
        }
    }
    closedir(dir);
}

void delete_show_file(const char *filename)
{
    if (!filename) return;
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s%s.x18s", SHOWS_DIR, filename);
    unlink(filepath);
    list_available_shows();
}

void duplicate_show_file(const char *src, const char *dst)
{
    if (!src || !dst) return;
    
    Show temp_show;
    if (!load_show_from_file(src, &temp_show)) return;
    
    strcpy(temp_show.name, dst);
    save_show_to_file(&temp_show);
    list_available_shows();
}

void rename_show_file(const char *old_name, const char *new_name)
{
    if (!old_name || !new_name) return;
    
    Show temp_show;
    if (!load_show_from_file(old_name, &temp_show)) return;
    
    delete_show_file(old_name);
    strcpy(temp_show.name, new_name);
    save_show_to_file(&temp_show);
    list_available_shows();
}

// ============================================================================
// MIXER FUNCTIONS
// ============================================================================

void init_mixer(void)
{
    for (int i = 0; i < NUM_FADERS; i++) {
        g_faders[i].id = i + 1;
        g_faders[i].value = 0.5f;
        g_faders[i].muted = 0;
        g_faders[i].eq_enabled = 0;
        g_faders[i].x = i * FADER_WIDTH;
        g_faders[i].y = 0;
        g_faders[i].w = FADER_WIDTH;
        g_faders[i].h = FADER_HEIGHT;
    }
}

int touch_hits_fader(touchPosition touch, Fader *fader, float *out_value)
{
    if (touch.px >= fader->x && touch.px < fader->x + fader->w) {
        float fader_top = 27;
        float fader_bottom = 200;
        
        if (touch.py >= fader_top && touch.py <= fader_bottom) {
            float fader_height = fader_bottom - fader_top;
            float pos = touch.py - fader_top;
            float val = 1.0f - (pos / fader_height);
            *out_value = (val < 0) ? 0 : (val > 1) ? 1 : val;
            return 1;
        }
    }
    return 0;
}

int touch_hits_mute_button(touchPosition touch, Fader *fader)
{
    return (touch.px >= fader->x && touch.px < fader->x + fader->w &&
            touch.py >= 210 && touch.py <= 227);
}

int touch_hits_eq_button(touchPosition touch, Fader *fader)
{
    return (touch.px >= fader->x && touch.px < fader->x + fader->w &&
            touch.py >= 5 && touch.py <= 22);
}

void update_mixer_touch(void)
{
    if (!g_isTouched) return;
    
    int touch_edge = g_isTouched && !g_wasTouched;
    
    for (int i = 0; i < NUM_FADERS; i++) {
        float val;
        
        if (touch_hits_fader(g_touchPos, &g_faders[i], &val)) {
            g_faders[i].value = val;
        }
        
        if (touch_edge) {
            if (touch_hits_mute_button(g_touchPos, &g_faders[i])) {
                g_faders[i].muted = 1 - g_faders[i].muted;
            }
            
            if (touch_hits_eq_button(g_touchPos, &g_faders[i])) {
                g_faders[i].eq_enabled = 1 - g_faders[i].eq_enabled;
            }
        }
    }
}

void update_touch_input(void)
{
    hidTouchRead(&g_touchPos);
    g_wasTouched = g_isTouched;
    g_isTouched = (hidKeysHeld() & KEY_TOUCH) ? 1 : 0;
    update_mixer_touch();
}

// ============================================================================
// GRAPHICS FUNCTIONS
// ============================================================================

void init_graphics(void)
{
    gfxInitDefault();
    
    // Mount RomFS for loading embedded assets (required before accessing romfs:/)
    romfsInit();
    
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    
    g_topScreen.target = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    g_topScreen.width = SCREEN_WIDTH_TOP;
    g_topScreen.height = SCREEN_HEIGHT_TOP;
    
    g_botScreen.target = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    g_botScreen.width = SCREEN_WIDTH_BOT;
    g_botScreen.height = SCREEN_HEIGHT_BOT;
    
    g_textBuf = C2D_TextBufNew(2048);
    
    // Load fader sprite sheets from /gfx/ (embedded in romfs)
    printf("[DEBUG] Attempting to load spritesheets from romfs:/gfx/\n");
    
    // List contents of gfx directory to verify romfs is mounted correctly
    DIR* dir = opendir("romfs:/gfx/");
    if (dir) {
        struct dirent* entry;
        printf("[DEBUG] Contents of romfs:/gfx/:\n");
        while ((entry = readdir(dir)) != NULL) {
            printf("[DEBUG]   - %s\n", entry->d_name);
        }
        closedir(dir);
    } else {
        printf("[DEBUG] ERROR: Could not open romfs:/gfx/ directory\n");
    }
    
    // Load grip sprite sheet
    g_grip_sheet = C2D_SpriteSheetLoad("romfs:/gfx/Grip.t3x");
    if (g_grip_sheet) {
        g_grip_img = C2D_SpriteSheetGetImage(g_grip_sheet, 0);
        g_grip_loaded = 1;
        printf("[DEBUG] Successfully loaded Grip.t3x\n");
    } else {
        printf("[DEBUG] Failed to load Grip.t3x\n");
    }
    
    // Load fader background sprite sheet
    g_fader_sheet = C2D_SpriteSheetLoad("romfs:/gfx/FaderBkg.t3x");
    if (g_fader_sheet) {
        g_fader_bkg = C2D_SpriteSheetGetImage(g_fader_sheet, 0);
        g_fader_loaded = 1;
        printf("[DEBUG] Successfully loaded FaderBkg.t3x\n");
    } else {
        printf("[DEBUG] Failed to load FaderBkg.t3x\n");
    }
    
    g_romfs_mounted = 1;
    
    init_mixer();
    init_default_show();
    apply_step_to_faders(0);
    
    // Test filesystem on startup
    test_filesystem_write();
    g_save_status_timer = 180;  // Show for 3 seconds on startup
}

void cleanup_graphics(void)
{
    // Free sprite sheets
    if (g_grip_sheet) {
        C2D_SpriteSheetFree(g_grip_sheet);
        g_grip_sheet = NULL;
    }
    if (g_fader_sheet) {
        C2D_SpriteSheetFree(g_fader_sheet);
        g_fader_sheet = NULL;
    }
    
    if (g_textBuf) {
        C2D_TextBufDelete(g_textBuf);
        g_textBuf = NULL;
    }
    C2D_Fini();
    C3D_Fini();
    
    // Unmount RomFS
    romfsExit();
    
    gfxExit();
}

void draw_rect(Screen* screen, float x, float y, float w, float h, u32 color)
{
    C2D_DrawRectSolid(x, y, 0.5f, w, h, color);
}

void draw_debug_text(Screen* screen, const char* text, float x, float y, float scale, u32 color)
{
    C2D_TextBufClear(g_textBuf);
    C2D_Text c2d_text;
    C2D_TextParse(&c2d_text, g_textBuf, text);
    C2D_TextOptimize(&c2d_text);
    C2D_DrawText(&c2d_text, C2D_WithColor, x, y, 0.5f, scale, scale, color);
}

// Draw a 3D button with shadow effect
void draw_3d_button(float x, float y, float w, float h, u32 color_main, u32 color_light, u32 color_dark, int pressed)
{
    // Shadow (bottom-right)
    C2D_DrawRectSolid(x + 2, y + 2, 0.4f, w, h, C2D_Color32(0x00, 0x00, 0x00, 0x60));
    
    // Main button body
    C2D_DrawRectSolid(x, y, 0.5f, w, h, color_main);
    
    if (!pressed) {
        // Top-left highlight
        C2D_DrawRectSolid(x, y, 0.51f, w - 1, 1, color_light);
        C2D_DrawRectSolid(x, y, 0.51f, 1, h - 1, color_light);
        
        // Bottom-right shadow
        C2D_DrawRectSolid(x + w - 1, y, 0.51f, 1, h, color_dark);
        C2D_DrawRectSolid(x, y + h - 1, 0.51f, w, 1, color_dark);
    } else {
        // Pressed: inverted highlights
        C2D_DrawRectSolid(x + w - 1, y, 0.51f, 1, h, color_light);
        C2D_DrawRectSolid(x, y + h - 1, 0.51f, w, 1, color_light);
    }
}

// ============================================================================
// RENDERING
// ============================================================================

void render_top_screen(void)
{
    u32 clrBgDark = C2D_Color32(0x1A, 0x1A, 0x1A, 0xFF);
    u32 clrBgMid = C2D_Color32(0x25, 0x25, 0x25, 0xFF);
    u32 clrBorder = C2D_Color32(0x50, 0x50, 0x50, 0xFF);
    u32 clrWhite = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    u32 clrYellow = C2D_Color32(0xFF, 0xFF, 0x00, 0xFF);
    u32 clrGreen = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);
    u32 clrCyan = C2D_Color32(0x00, 0xFF, 0xFF, 0xFF);
    u32 clrRed = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
    
    C2D_TargetClear(g_topScreen.target, clrBgDark);
    C2D_SceneBegin(g_topScreen.target);
    
    if (g_app_mode == APP_MODE_MIXER) {
        if (g_creating_new_show) {
            // ===== SIMPLE INPUT DISPLAY =====
            C2D_DrawRectSolid(0, 0, 0.5f, SCREEN_WIDTH_TOP, 50, C2D_Color32(0x0F, 0x0F, 0x3F, 0xFF));
            C2D_DrawRectangle(0, 0, 0.5f, SCREEN_WIDTH_TOP, 50, clrBorder, clrBorder, clrBorder, clrBorder);
            draw_debug_text(&g_topScreen, "Enter Show Name", 15.0f, 10.0f, 0.55f, clrYellow);
            draw_debug_text(&g_topScreen, g_new_show_name, 15.0f, 30.0f, 0.50f, clrCyan);
            
            // Rest of top screen empty
            C2D_DrawRectSolid(0, 50, 0.5f, SCREEN_WIDTH_TOP, SCREEN_HEIGHT_TOP - 50, clrBgDark);
        } else {
            // ===== TITLE BAR =====
            C2D_DrawRectSolid(0, 0, 0.5f, SCREEN_WIDTH_TOP, 35, C2D_Color32(0x0F, 0x0F, 0x3F, 0xFF));
            C2D_DrawRectangle(0, 0, 0.5f, SCREEN_WIDTH_TOP, 35, clrBorder, clrBorder, clrBorder, clrBorder);
            draw_debug_text(&g_topScreen, g_current_show.name, 15.0f, 8.0f, 0.55f, clrYellow);
            
            // Show step count
            char step_count_str[32];
            snprintf(step_count_str, sizeof(step_count_str), "Steps: %d", g_current_show.num_steps);
            draw_debug_text(&g_topScreen, step_count_str, SCREEN_WIDTH_TOP - 150, 8.0f, 0.45f, clrWhite);
            
            // ===== STEPS LISTBOX =====
            // Listbox background
            C2D_DrawRectSolid(0, 35, 0.5f, SCREEN_WIDTH_TOP, 160, clrBgMid);
            C2D_DrawRectangle(0, 35, 0.5f, SCREEN_WIDTH_TOP, 160, clrBorder, clrBorder, clrBorder, clrBorder);
            
            // Draw steps (max 8 visible at a time)
            int start_idx = g_selected_step - 3;  // Show selected step + context
            if (start_idx < 0) start_idx = 0;
            if (start_idx + 8 > g_current_show.num_steps) {
                start_idx = g_current_show.num_steps - 8;
                if (start_idx < 0) start_idx = 0;
            }
            
            float list_y = 42.0f;
            for (int i = start_idx; i < start_idx + 8 && i < g_current_show.num_steps; i++) {
                u32 step_color = clrWhite;
                
                // Highlight selected step
                if (i == g_selected_step) {
                    C2D_DrawRectSolid(5, list_y - 2, 0.5f, SCREEN_WIDTH_TOP - 10, 20, C2D_Color32(0x00, 0x44, 0x88, 0xFF));
                    step_color = clrYellow;
                }
                
                char step_text[80];
                snprintf(step_text, sizeof(step_text), "[%3d] %-25s", i + 1, g_current_show.steps[i].name);
                draw_debug_text(&g_topScreen, step_text, 15.0f, list_y, 0.40f, step_color);
                
                list_y += 20.0f;
            }
            
            // ===== BOTTOM INFO BOXES =====
            // Left box - Messages
            C2D_DrawRectSolid(0, 195, 0.5f, 200, 45, clrBgMid);
            C2D_DrawRectangle(0, 195, 0.5f, 200, 45, clrBorder, clrBorder, clrBorder, clrBorder);
            draw_debug_text(&g_topScreen, "Message:", 8.0f, 198.0f, 0.35f, clrCyan);
            
            if (g_save_status_timer > 0) {
                u32 msg_color = clrWhite;
                if (strstr(g_save_status, "ERROR")) {
                    msg_color = clrRed;
                } else if (strstr(g_save_status, "SAVED")) {
                    msg_color = clrGreen;
                } else if (strstr(g_save_status, "Added") || strstr(g_save_status, "Duplicated")) {
                    msg_color = clrYellow;
                } else if (strstr(g_save_status, "OK")) {
                    msg_color = clrGreen;
                }
                draw_debug_text(&g_topScreen, g_save_status, 8.0f, 213.0f, 0.35f, msg_color);
                g_save_status_timer--;
            } else {
                draw_debug_text(&g_topScreen, "Ready", 8.0f, 213.0f, 0.35f, clrWhite);
            }
            
            // Right box - Debug info
            C2D_DrawRectSolid(200, 195, 0.5f, 200, 45, clrBgMid);
            C2D_DrawRectangle(200, 195, 0.5f, 200, 45, clrBorder, clrBorder, clrBorder, clrBorder);
            draw_debug_text(&g_topScreen, "Info:", 208.0f, 198.0f, 0.35f, clrCyan);
            
            char info_str[80];
            snprintf(info_str, sizeof(info_str), "Step %d/%d | Vol: %d%%", 
                     g_selected_step + 1, g_current_show.num_steps,
                     (int)(g_faders[0].value * 100));
            draw_debug_text(&g_topScreen, info_str, 208.0f, 213.0f, 0.35f, clrWhite);
        }
    }
}

void render_keyboard(void)
{
    u32 clrBg = C2D_Color32(0x20, 0x20, 0x20, 0xFF);
    u32 clrBorder = C2D_Color32(0x50, 0x50, 0x50, 0xFF);
    u32 clrWhite = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    u32 clrRed = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
    u32 clrGreen = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);
    u32 clrYellow = C2D_Color32(0xFF, 0xFF, 0x00, 0xFF);
    u32 clrCyan = C2D_Color32(0x00, 0xFF, 0xFF, 0xFF);
    
    C2D_TargetClear(g_botScreen.target, clrBg);
    C2D_SceneBegin(g_botScreen.target);
    
    // Title
    draw_debug_text(&g_botScreen, "Virtual Keyboard", 10, 5, 0.4f, clrCyan);
    
    // Input display
    C2D_DrawRectangle(10, 20, 0.5f, SCREEN_WIDTH_BOT - 20, 30, clrBorder, clrBorder, clrBorder, clrBorder);
    draw_debug_text(&g_botScreen, g_new_show_name, 15, 25, 0.4f, clrYellow);
    
    // Virtual keyboard rows
    float kb_y = 55.0f;
    float key_width = (SCREEN_WIDTH_BOT - 20) / 10.0f;
    
    // Row 1: QWERTYUIOP
    const char *row1 = "QWERTYUIOP";
    for (int i = 0; i < 10; i++) {
        char key_char[2] = {row1[i], '\0'};
        float key_x = 10.0f + i * key_width;
        C2D_DrawRectangle(key_x, kb_y, 0.5f, key_width - 2, 20, clrBorder, clrBorder, clrBorder, clrBorder);
        draw_debug_text(&g_botScreen, key_char, key_x + 4, kb_y + 2, 0.35f, clrWhite);
    }
    
    // Row 2: ASDFGHJKL
    const char *row2 = "ASDFGHJKL";
    kb_y += 22.0f;
    for (int i = 0; i < 9; i++) {
        char key_char[2] = {row2[i], '\0'};
        float key_x = 10.0f + (i + 0.5f) * key_width;
        C2D_DrawRectangle(key_x, kb_y, 0.5f, key_width - 2, 20, clrBorder, clrBorder, clrBorder, clrBorder);
        draw_debug_text(&g_botScreen, key_char, key_x + 4, kb_y + 2, 0.35f, clrWhite);
    }
    
    // Row 3: ZXCVBNM
    const char *row3 = "ZXCVBNM";
    kb_y += 22.0f;
    for (int i = 0; i < 7; i++) {
        char key_char[2] = {row3[i], '\0'};
        float key_x = 10.0f + (i + 1.5f) * key_width;
        C2D_DrawRectangle(key_x, kb_y, 0.5f, key_width - 2, 20, clrBorder, clrBorder, clrBorder, clrBorder);
        draw_debug_text(&g_botScreen, key_char, key_x + 4, kb_y + 2, 0.35f, clrWhite);
    }
    
    // Row 4: Action buttons
    kb_y += 22.0f;
    float btn_width = (SCREEN_WIDTH_BOT - 20) / 4.0f;
    
    // Backspace
    C2D_DrawRectangle(10, kb_y, 0.5f, btn_width - 2, 20, clrBorder, clrBorder, clrBorder, clrBorder);
    draw_debug_text(&g_botScreen, "<--", 15, kb_y + 2, 0.3f, clrRed);
    
    // Space
    C2D_DrawRectangle(10 + btn_width, kb_y, 0.5f, btn_width - 2, 20, clrBorder, clrBorder, clrBorder, clrBorder);
    draw_debug_text(&g_botScreen, "SPC", 10 + btn_width + 5, kb_y + 2, 0.3f, clrWhite);
    
    // OK
    C2D_DrawRectangle(10 + btn_width * 2, kb_y, 0.5f, btn_width - 2, 20, clrBorder, clrBorder, clrBorder, clrBorder);
    draw_debug_text(&g_botScreen, "OK", 10 + btn_width * 2 + 12, kb_y + 2, 0.35f, clrGreen);
    
    // Cancel
    C2D_DrawRectangle(10 + btn_width * 3, kb_y, 0.5f, btn_width - 2, 20, clrBorder, clrBorder, clrBorder, clrBorder);
    draw_debug_text(&g_botScreen, "Esc", 10 + btn_width * 3 + 10, kb_y + 2, 0.3f, clrRed);
}

void render_bot_screen(void)
{
    if (g_creating_new_show) {
        render_keyboard();
        return;
    }
    
    u32 clrBg = C2D_Color32(0x1A, 0x1A, 0x1A, 0xFF);
    C2D_TargetClear(g_botScreen.target, clrBg);
    C2D_SceneBegin(g_botScreen.target);
    
    // Debug: Show image loading status
    char debug_msg[128];
    snprintf(debug_msg, sizeof(debug_msg), "RomFS:%d Grip:%d Fader:%d", 
             g_romfs_mounted, g_grip_loaded, g_fader_loaded);
    draw_debug_text(&g_botScreen, debug_msg, 5, 5, 0.25f, C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF));
    
    u32 clrMuteMain = C2D_Color32(0xDD, 0x33, 0x33, 0xFF);
    u32 clrMuteLight = C2D_Color32(0xFF, 0x66, 0x66, 0xFF);
    u32 clrMuteDark = C2D_Color32(0x88, 0x00, 0x00, 0xFF);
    u32 clrEqMain = C2D_Color32(0x33, 0xDD, 0x33, 0xFF);
    u32 clrEqLight = C2D_Color32(0x66, 0xFF, 0x66, 0xFF);
    u32 clrEqDark = C2D_Color32(0x00, 0x88, 0x00, 0xFF);
    u32 clrText = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    u32 clrBorder = C2D_Color32(0x40, 0x40, 0x40, 0xFF);
    u32 clrGripMain = C2D_Color32(0xCC, 0xCC, 0xCC, 0xFF);  // Light gray grip
    u32 clrGripLight = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);  // White highlight
    u32 clrGripDark = C2D_Color32(0x66, 0x66, 0x66, 0xFF);   // Dark gray shadow
    u32 clrFaderTrack = C2D_Color32(0x1A, 0x1A, 0x1A, 0xFF);  // Very dark track
    u32 clrTickMark = C2D_Color32(0x55, 0x55, 0x55, 0xFF);
    
    for (int i = 0; i < NUM_FADERS; i++) {
        Fader *f = &g_faders[i];
        
        // Channel border
        C2D_DrawRectangle(f->x, f->y, 0.5f, f->w, f->h, clrBorder, clrBorder, clrBorder, clrBorder);
        
        // EQ button at top (NEW POSITION)
        u32 eq_color_main = f->eq_enabled ? clrEqDark : clrEqMain;
        u32 eq_color_light = f->eq_enabled ? C2D_Color32(0x88, 0xFF, 0x88, 0xFF) : clrEqLight;
        u32 eq_color_dark = f->eq_enabled ? clrEqLight : clrEqDark;
        draw_3d_button(f->x + 1, 5, f->w - 2, 14, eq_color_main, eq_color_light, eq_color_dark, f->eq_enabled);
        draw_debug_text(&g_botScreen, "Eq", f->x + 2, 7, 0.25f, clrText);
        
        // Channel number (below EQ button)
        char label[4];
        snprintf(label, sizeof(label), "%d", f->id);
        draw_debug_text(&g_botScreen, label, f->x + 2, 22, 0.25f, clrText);
        
        // Volume percentage
        char vol_str[8];
        snprintf(vol_str, sizeof(vol_str), "%d%%", (int)(f->value * 100));
        draw_debug_text(&g_botScreen, vol_str, f->x + 1, 32, 0.2f, clrText);
        
        // ===== FADER TRACK WITH SCALE MARKS =====
        float fader_top = 45;
        float fader_bottom = 205;
        float fader_height = fader_bottom - fader_top;
        float bar_x = f->x + (f->w - FADER_BAR_WIDTH) / 2;
        
        // Draw fader background image if loaded, otherwise use procedural fallback
        if (g_fader_bkg.tex != NULL) {
            // Draw fader background image scaled to fit the fader area
            // Image is 94x368, we need to scale to approximately 6-8 width x fader_height
            float img_scale_x = 6.0f / 94.0f;
            float img_scale_y = fader_height / 368.0f;
            C2D_DrawImageAt(g_fader_bkg,
                           bar_x - 2 + (4 - 6) * 0.5f, fader_top,  // x, y (centered)
                           0.5f, NULL, img_scale_x, img_scale_y);
        } else {
            // Fallback: procedural fader track
            C2D_DrawRectSolid(bar_x - 2, fader_top, 0.5f, FADER_BAR_WIDTH + 4, fader_height, clrFaderTrack);
            C2D_DrawRectangle(bar_x - 2, fader_top, 0.5f, FADER_BAR_WIDTH + 4, fader_height, 
                             clrBorder, clrBorder, clrBorder, clrBorder);
            
            // Draw tick marks
            const float tick_positions[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
            for (int tick = 0; tick < 5; tick++) {
                float tick_y = fader_top + (fader_height * tick_positions[tick]);
                C2D_DrawRectSolid(bar_x - 2, tick_y, 0.51f, 3, 1, clrTickMark);
                for (int minor = 1; minor < 4; minor++) {
                    float minor_tick_y = tick_y + (fader_height * 0.25f * (minor / 4.0f));
                    if (minor_tick_y < fader_bottom) {
                        C2D_DrawRectSolid(bar_x - 1, minor_tick_y, 0.51f, 2, 1, C2D_Color32(0x33, 0x33, 0x33, 0xFF));
                    }
                }
            }
        }
        
        // ===== GRIP/SLIDER =====
        float grip_y = fader_bottom - (fader_height * f->value);
        float grip_x = f->x + (f->w - 11) / 2;  // Center the grip
        
        // Draw grip image if loaded, otherwise use procedural fallback
        if (g_grip_img.tex != NULL) {
            // Draw grip image scaled to appropriate size
            // Original is 38x68, we want approximately 12x16
            float grip_scale_x = 12.0f / 38.0f;
            float grip_scale_y = 16.0f / 68.0f;
            C2D_DrawImageAt(g_grip_img,
                           grip_x - (12 - 11) * 0.5f, grip_y,  // Center grip over track
                           0.5f, NULL, grip_scale_x, grip_scale_y);
        } else {
            // Fallback: procedural grip
            float grip_w = 11;
            float grip_h = 16;
            
            C2D_DrawRectSolid(grip_x + 1, grip_y + 1, 0.4f, grip_w, grip_h, C2D_Color32(0x00, 0x00, 0x00, 0x90));
            C2D_DrawRectSolid(grip_x, grip_y, 0.5f, grip_w, grip_h, clrGripMain);
            C2D_DrawRectSolid(grip_x + 1, grip_y + 1, 0.51f, grip_w - 2, 2, clrGripLight);
            C2D_DrawRectSolid(grip_x, grip_y + 2, 0.51f, 1, grip_h - 4, clrGripLight);
            C2D_DrawRectSolid(grip_x + grip_w - 1, grip_y + 2, 0.51f, 1, grip_h - 4, clrGripDark);
            C2D_DrawRectSolid(grip_x + grip_w / 2, grip_y + 3, 0.52f, 1, grip_h - 6, C2D_Color32(0xAA, 0xAA, 0xAA, 0xFF));
            C2D_DrawRectangle(grip_x, grip_y, 0.52f, grip_w, grip_h, clrGripDark, clrGripDark, clrGripDark, clrGripDark);
        }
        
        // Mute button at bottom (NEW POSITION)
        // Dark when not muted (sollevato), bright when muted (acceso/premuto)
        u32 mute_color_main = f->muted ? clrMuteMain : clrMuteDark;
        u32 mute_color_light = f->muted ? clrMuteLight : C2D_Color32(0x55, 0x00, 0x00, 0xFF);
        u32 mute_color_dark = f->muted ? clrMuteDark : C2D_Color32(0x44, 0x00, 0x00, 0xFF);
        draw_3d_button(f->x + 1, 210, f->w - 2, 17, mute_color_main, mute_color_light, mute_color_dark, f->muted);
        draw_debug_text(&g_botScreen, "M", f->x + 4, 212, 0.25f, clrText);
    }
}

void render_show_manager(void)
{
    u32 clrBg = C2D_Color32(0x20, 0x20, 0x20, 0xFF);
    C2D_TargetClear(g_botScreen.target, clrBg);
    C2D_SceneBegin(g_botScreen.target);
    
    u32 clrText = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    u32 clrSelected = C2D_Color32(0xFF, 0xFF, 0x00, 0xFF);
    u32 clrBorder = C2D_Color32(0x50, 0x50, 0x50, 0xFF);
    u32 clrBtnBg = C2D_Color32(0x40, 0x40, 0x40, 0xFF);
    
    // Title
    draw_debug_text(&g_botScreen, "Show Manager", 5, 5, 0.4f, clrText);
    
    // List of shows (starts at y=25)
    float list_y = 25.0f;
    u32 clrNotSaved = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);  // Red for unsaved
    
    for (int i = 0; i < g_num_available_shows && i < 10; i++) {
        u32 color = (i == g_selected_show) ? clrSelected : clrText;
        
        // Check if this show is the current one and not saved
        if (g_show_modified && strcmp(g_available_shows[i], g_current_show.name) == 0) {
            color = clrNotSaved;
        }
        
        // Show item background
        if (i == g_selected_show) {
            C2D_DrawRectSolid(0, list_y, 0.5f, SCREEN_WIDTH_BOT, 18, C2D_Color32(0x30, 0x30, 0x60, 0xFF));
        }
        
        // Check if renaming this item
        if (g_renaming && i == g_selected_show) {
            draw_debug_text(&g_botScreen, g_new_name, 10, list_y + 2, 0.35f, clrSelected);
            draw_debug_text(&g_botScreen, "_", 10 + g_rename_input_pos * 8, list_y + 2, 0.35f, clrSelected);
        } else {
            draw_debug_text(&g_botScreen, g_available_shows[i], 10, list_y + 2, 0.35f, color);
        }
        
        list_y += 18.0f;
    }
    
    // Bottom buttons (now 5: LOAD, DEL, DUP, REN, EXIT)
    float btn_y = 210.0f;
    float btn_width = SCREEN_WIDTH_BOT / 5.0f;
    const char *btn_labels[] = {"LOAD", "DEL", "DUP", "REN", "EXIT"};
    u32 clrExit = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);  // Red for exit button
    
    for (int i = 0; i < 5; i++) {
        float btn_x = i * btn_width;
        C2D_DrawRectSolid(btn_x, btn_y, 0.5f, btn_width - 1, 30, clrBtnBg);
        C2D_DrawRectangle(btn_x, btn_y, 0.5f, btn_width - 1, 30, clrBorder, clrBorder, clrBorder, clrBorder);
        
        u32 btn_text_color = (i == 4) ? clrExit : clrText;  // Red for EXIT
        draw_debug_text(&g_botScreen, btn_labels[i], btn_x + 5, btn_y + 6, 0.28f, btn_text_color);
    }
}

int check_button_touch(int button_idx)
{
    if (!g_isTouched) return 0;
    
    float btn_width = SCREEN_WIDTH_BOT / 5.0f;  // Now 5 buttons
    float btn_x = button_idx * btn_width;
    float btn_y = 210.0f;
    
    return (g_touchPos.px >= btn_x && g_touchPos.px < btn_x + btn_width &&
            g_touchPos.py >= btn_y && g_touchPos.py < btn_y + 30);
}

int get_show_item_from_touch(void)
{
    if (!g_isTouched) return -1;
    
    float list_y = 25.0f;
    for (int i = 0; i < g_num_available_shows && i < 10; i++) {
        if (g_touchPos.py >= list_y && g_touchPos.py < list_y + 18) {
            return i;
        }
        list_y += 18.0f;
    }
    return -1;
}

// Forward declaration for keyboard input handling
void handle_keyboard_input(char c);

void handle_new_show_input(void)
{
    u32 kDown = hidKeysDown();
    int touch_edge = g_isTouched && !g_wasTouched;
    
    // Handle button press input (A/B for quick confirm/cancel)
    if (kDown & KEY_A) {
        // Confirm
        if (strlen(g_new_show_name) > 0) {
            g_new_show_name[63] = '\0';  // Ensure null termination
            init_new_show(g_new_show_name);  // Create new show with 3 default steps
            save_show_to_file(&g_current_show);  // Save immediately
            
            snprintf(g_save_status, sizeof(g_save_status), "OK: '%s' created", g_new_show_name);
            g_save_status_timer = 120;
            
            g_creating_new_show = 0;
            memset(g_new_show_name, 0, sizeof(g_new_show_name));
            list_available_shows();
        }
        return;
    } else if (kDown & KEY_B) {
        // Cancel
        g_creating_new_show = 0;
        memset(g_new_show_name, 0, sizeof(g_new_show_name));
        return;
    }
    
    // Handle touch input on virtual keyboard
    if (touch_edge) {
        float touch_x = (float)g_touchPos.px;
        float touch_y = (float)g_touchPos.py;
        
        // Keyboard layout on bottom screen
        float key_width = (SCREEN_WIDTH_BOT - 20.0f) / 10.0f;
        
        // Row 1: QWERTYUIOP (y=55 to 75)
        if (touch_y >= 55.0f && touch_y < 75.0f) {
            if (touch_x >= 10.0f && touch_x < SCREEN_WIDTH_BOT - 10.0f) {
                int key_idx = (int)((touch_x - 10.0f) / key_width);
                if (key_idx >= 0 && key_idx < 10) {
                    const char *row1 = "QWERTYUIOP";
                    char key_char[2] = {row1[key_idx], '\0'};
                    handle_keyboard_input(key_char[0]);
                }
            }
        }
        // Row 2: ASDFGHJKL (y=77 to 97)
        else if (touch_y >= 77.0f && touch_y < 97.0f) {
            if (touch_x >= 10.0f && touch_x < SCREEN_WIDTH_BOT - 10.0f) {
                int key_idx = (int)((touch_x - 10.0f) / key_width);
                if (key_idx >= 0 && key_idx < 9) {
                    const char *row2 = "ASDFGHJKL";
                    char key_char[2] = {row2[key_idx], '\0'};
                    handle_keyboard_input(key_char[0]);
                }
            }
        }
        // Row 3: ZXCVBNM (y=99 to 119)
        else if (touch_y >= 99.0f && touch_y < 119.0f) {
            if (touch_x >= 10.0f && touch_x < SCREEN_WIDTH_BOT - 10.0f) {
                int key_idx = (int)((touch_x - 10.0f) / key_width);
                if (key_idx >= 1 && key_idx < 8) {  // Offset by 1.5
                    const char *row3 = "ZXCVBNM";
                    char key_char[2] = {row3[key_idx - 1], '\0'};
                    handle_keyboard_input(key_char[0]);
                }
            }
        }
        // Row 4: Action buttons (y=121 to 141)
        else if (touch_y >= 121.0f && touch_y < 141.0f) {
            float btn_width = (SCREEN_WIDTH_BOT - 20.0f) / 4.0f;
            
            // Backspace (x=10 to btn_width+10)
            if (touch_x >= 10.0f && touch_x < 10.0f + btn_width) {
                // Delete last character
                if (strlen(g_new_show_name) > 0) {
                    g_new_show_name[strlen(g_new_show_name) - 1] = '\0';
                }
            }
            // Space (x=btn_width+10 to 2*btn_width+10)
            else if (touch_x >= 10.0f + btn_width && touch_x < 10.0f + btn_width * 2) {
                handle_keyboard_input(' ');
            }
            // OK (x=2*btn_width+10 to 3*btn_width+10)
            else if (touch_x >= 10.0f + btn_width * 2 && touch_x < 10.0f + btn_width * 3) {
                // Confirm
                if (strlen(g_new_show_name) > 0) {
                    g_new_show_name[63] = '\0';
                    init_new_show(g_new_show_name);  // Create new show with 3 default steps
                    save_show_to_file(&g_current_show);  // Save immediately
                    
                    snprintf(g_save_status, sizeof(g_save_status), "OK: '%s' created", g_new_show_name);
                    g_save_status_timer = 120;
                    
                    g_creating_new_show = 0;
                    memset(g_new_show_name, 0, sizeof(g_new_show_name));
                }
            }
            // Cancel (x=3*btn_width+10 to 4*btn_width+10)
            else if (touch_x >= 10.0f + btn_width * 3 && touch_x < 10.0f + btn_width * 4) {
                g_creating_new_show = 0;
                memset(g_new_show_name, 0, sizeof(g_new_show_name));
            }
        }
    }
}

void handle_keyboard_input(char c)
{
    size_t len = strlen(g_new_show_name);
    if (len < 63) {
        g_new_show_name[len] = c;
        g_new_show_name[len + 1] = '\0';
    }
}

void handle_manager_input(void)
{
    u32 kDown = hidKeysDown();
    int touch_edge = g_isTouched && !g_wasTouched;
    
    if (g_renaming) {
        // Handle renaming input
        if (kDown & KEY_A) {
            // Confirm rename
            if (g_rename_input_pos > 0) {
                g_new_name[g_rename_input_pos] = '\0';
                rename_show_file(g_available_shows[g_selected_show], g_new_name);
                g_renaming = 0;
                g_rename_input_pos = 0;
            }
        } else if (kDown & KEY_B) {
            // Cancel rename
            g_renaming = 0;
            g_rename_input_pos = 0;
        } else if (kDown & KEY_DRIGHT) {
            if (g_rename_input_pos < 60) {
                g_new_name[g_rename_input_pos] = 'A' + (g_rename_input_pos % 26);
                g_rename_input_pos++;
            }
        } else if (kDown & KEY_DLEFT) {
            if (g_rename_input_pos > 0) {
                g_rename_input_pos--;
            }
        }
    } else {
        // Regular manager input
        if (kDown & KEY_DUP) {
            g_selected_show--;
            if (g_selected_show < 0) g_selected_show = g_num_available_shows - 1;
        } else if (kDown & KEY_DDOWN) {
            g_selected_show++;
            if (g_selected_show >= g_num_available_shows) g_selected_show = 0;
        } else if (kDown & KEY_B) {
            g_app_mode = APP_MODE_MIXER;
        }
        
        // Touch buttons
        if (touch_edge) {
            int touched_show = get_show_item_from_touch();
            if (touched_show >= 0) {
                g_selected_show = touched_show;
            }
            
            if (check_button_touch(BTN_LOAD)) {
                if (g_selected_show >= 0 && g_selected_show < g_num_available_shows) {
                    if (load_show_from_file(g_available_shows[g_selected_show], &g_current_show)) {
                        g_show_loaded = 1;
                        g_selected_step = 0;
                        apply_step_to_faders(0);
                        g_app_mode = APP_MODE_MIXER;
                    }
                }
            } else if (check_button_touch(BTN_DELETE)) {
                if (g_selected_show >= 0 && g_selected_show < g_num_available_shows) {
                    delete_show_file(g_available_shows[g_selected_show]);
                    if (g_selected_show >= g_num_available_shows) {
                        g_selected_show = g_num_available_shows - 1;
                    }
                }
            } else if (check_button_touch(BTN_DUPLICATE)) {
                if (g_selected_show >= 0 && g_selected_show < g_num_available_shows) {
                    char new_name[64];
                    snprintf(new_name, sizeof(new_name), "%s_copy", g_available_shows[g_selected_show]);
                    duplicate_show_file(g_available_shows[g_selected_show], new_name);
                }
            } else if (check_button_touch(BTN_RENAME)) {
                if (g_selected_show >= 0 && g_selected_show < g_num_available_shows) {
                    strcpy(g_new_name, g_available_shows[g_selected_show]);
                    g_rename_input_pos = strlen(g_new_name);
                    g_renaming = 1;
                }
            } else if (check_button_touch(4)) {  // Button 4 = EXIT
                // EXIT button - save if modified, then close
                if (g_show_modified) {
                    save_show_to_file(&g_current_show);
                }
                // Signal that we should close the app
                g_should_exit = 1;
            }
        }
    }
}

void render_frame(void)
{
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    render_top_screen();
    
    if (g_app_mode == APP_MODE_MIXER) {
        render_bot_screen();  // Always show mixer on bottom screen
    } else {
        render_show_manager();
    }
    
    C3D_FrameEnd(0);
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char* argv[])
{
    init_graphics();
    
    while (aptMainLoop())
    {
        hidScanInput();
        update_touch_input();
        
        u32 kDown = hidKeysDown();
        
        // START button opens/closes show manager
        if (kDown & KEY_START) {
            if (g_app_mode == APP_MODE_MIXER) {
                // Opening manager: do NOT save, just switch
                list_available_shows();
                g_app_mode = APP_MODE_MANAGER;
            } else {
                // Closing manager: return to mixer
                g_app_mode = APP_MODE_MIXER;
            }
        }
        
        if (g_app_mode == APP_MODE_MIXER) {
            if (g_creating_new_show) {
                // Handle new show naming input
                handle_new_show_input();
            } else {
                // SELECT button: Start creating new show
                if (kDown & KEY_SELECT) {
                    g_creating_new_show = 1;
                    g_new_show_input_pos = 0;
                    memset(g_new_show_name, 0, sizeof(g_new_show_name));
                }
                
                // Up/Down: Navigate steps
                if (kDown & KEY_DUP) {
                    g_selected_step--;
                    if (g_selected_step < 0) g_selected_step = g_current_show.num_steps - 1;
                    apply_step_to_faders(g_selected_step);
                }
                if (kDown & KEY_DDOWN) {
                    g_selected_step++;
                    if (g_selected_step >= g_current_show.num_steps) g_selected_step = 0;
                    apply_step_to_faders(g_selected_step);
                }
                
                // X: Save show
                if (kDown & KEY_X) {
                    save_show_to_file(&g_current_show);
                }
                
                // Y: Save step
                if (kDown & KEY_Y) {
                    save_step_from_faders(g_selected_step);
                    save_show_to_file(&g_current_show);
                }
                
                // L: Add new step
                if (kDown & KEY_L) {
                    add_step();
                }
                
                // R: Duplicate current step
                if (kDown & KEY_R) {
                    duplicate_step();
                }
            }
        } else {
            // Show Manager mode
            handle_manager_input();
        }
        
        render_frame();
        gspWaitForVBlank();
        
        // Check if we should exit the app
        if (g_should_exit) {
            break;
        }
    }
    
    cleanup_graphics();
    return 0;}