#include "common.h"
#include "options_window.h"
#include <unistd.h>

// ============================================================================
// GLOBAL STATE
// ============================================================================

Options g_options = {1, 1};  // Default: both enabled
int g_options_window_open = 0;
int g_options_selected_checkbox = 0;  // 0=fader, 1=eq

#define OPTIONS_FILE "/3ds/x18mixer/Options"

// ============================================================================
// WINDOW LAYOUT CONSTANTS
// ============================================================================

#define WIN_WIDTH 240.0f
#define WIN_HEIGHT 180.0f
#define WIN_X ((SCREEN_WIDTH_BOT - WIN_WIDTH) / 2.0f)
#define WIN_Y ((SCREEN_HEIGHT_BOT - WIN_HEIGHT) / 2.0f)

#define CHECKBOX_SIZE 22.0f
#define CHECKBOX_X (WIN_X + 20.0f)
#define LABEL_WIDTH 50.0f

#define CHECKBOX1_Y (WIN_Y + 50.0f)
#define CHECKBOX2_Y (WIN_Y + 105.0f)

// ============================================================================
// COLOR PALETTE
// ============================================================================

#define CLR_BG_WINDOW C2D_Color32(0x30, 0x30, 0x50, 0xFF)  // Dark blue
#define CLR_BORDER C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF)      // White
#define CLR_TITLE C2D_Color32(0x80, 0xFF, 0xFF, 0xFF)       // Cyan
#define CLR_LABEL C2D_Color32(0xFF, 0xFF, 0x00, 0xFF)       // Yellow
#define CLR_ENABLED C2D_Color32(0x00, 0xFF, 0x00, 0xFF)     // Green
#define CLR_DISABLED C2D_Color32(0x44, 0x44, 0x88, 0xFF)    // Blue
#define CLR_CHECKMARK C2D_Color32(0x00, 0xFF, 0x00, 0xFF)   // Green
#define CLR_X C2D_Color32(0xFF, 0x00, 0x00, 0xFF)           // Red

// ============================================================================
// FILE I/O FUNCTIONS
// ============================================================================

void init_options(void)
{
    g_options.send_fader = 1;
    g_options.send_eq = 1;
    g_options_selected_checkbox = 0;
}

void load_options(void)
{
    FILE *f = fopen(OPTIONS_FILE, "r");
    if (!f) {
        init_options();
        return;
    }
    
    char line[256];
    char section[64] = {0};
    
    while (fgets(line, sizeof(line), f)) {
        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#' || line[0] == ';') continue;
        
        // Parse section header [SECTION]
        if (line[0] == '[') {
            int len = 0;
            for (int i = 1; line[i] != ']' && line[i] != '\0' && len < 63; i++) {
                section[len++] = line[i];
            }
            section[len] = '\0';
            continue;
        }
        
        // Parse key=value pairs
        if (strcmp(section, "OSC_SEND") == 0) {
            char key[32], value[32];
            if (sscanf(line, "%31[^=]=%31s", key, value) == 2) {
                if (strcmp(key, "fader") == 0) {
                    g_options.send_fader = atoi(value);
                } else if (strcmp(key, "eq") == 0) {
                    g_options.send_eq = atoi(value);
                }
            }
        }
    }
    
    fclose(f);
}

void save_options(void)
{
    FILE *f = fopen(OPTIONS_FILE, "w");
    if (!f) return;
    
    fprintf(f, "# X18 Mixer Options\n");
    fprintf(f, "# Auto-generated - do not edit manually\n\n");
    fprintf(f, "[OSC_SEND]\n");
    fprintf(f, "fader=%d\n", g_options.send_fader);
    fprintf(f, "eq=%d\n", g_options.send_eq);
    
    fflush(f);
    fsync(fileno(f));
    fclose(f);
}

// ============================================================================
// RENDERING HELPERS
// ============================================================================

// Draw a single checkbox with label and status indicator
static void draw_checkbox_item(float x, float y, const char *label_text, int is_enabled, int is_selected)
{
    // Draw checkbox background
    u32 checkbox_color = is_enabled ? CLR_BORDER_GREEN : CLR_X;
    C2D_DrawRectSolid(x, y, 0.29f, CHECKBOX_SIZE, CHECKBOX_SIZE, checkbox_color);
    
    // Draw 3D checkbox border
    draw_3d_border(x, y, CHECKBOX_SIZE, CHECKBOX_SIZE, CLR_BORDER_BRIGHT, CLR_SHADOW_BLACK, 2);
    
    // Draw status symbol (✓ or ✗)
    const char *symbol = is_enabled ? "✓" : "✗";
    u32 symbol_color = is_enabled ? CLR_BORDER_GREEN : CLR_X;
    draw_debug_text(&g_botScreen, symbol, x + 4, y + 2, 0.8f, symbol_color);
    
    // Draw label to the right of checkbox
    draw_debug_text(&g_botScreen, label_text, x + CHECKBOX_SIZE + 10, y + 4, 0.55f, CLR_TEXT_PRIMARY);
    
    // Draw selection indicator (underline with bright color)
    if (is_selected) {
        C2D_DrawRectSolid(x, y + CHECKBOX_SIZE + 3, 0.28f, CHECKBOX_SIZE + 100, 2, CLR_BORDER_CYAN);
    }
}

// ============================================================================
// MAIN RENDERING FUNCTION
// ============================================================================

void render_options_window(void)
{
    if (!g_options_window_open) return;
    
    // Draw window background and borders with 3D effect
    C2D_DrawRectSolid(WIN_X, WIN_Y, 0.30f, WIN_WIDTH, WIN_HEIGHT, CLR_BG_SECONDARY);
    draw_3d_border(WIN_X, WIN_Y, WIN_WIDTH, WIN_HEIGHT, CLR_BORDER_BRIGHT, CLR_SHADOW_BLACK, 2);
    
    // Draw title with header style
    draw_panel_header(WIN_X + 2, WIN_Y + 2, WIN_WIDTH - 4, 35.0f, "Options", CLR_BORDER_CYAN);
    draw_debug_text(&g_botScreen, "OSC Options", WIN_X + 20, WIN_Y + 12, 0.55f, CLR_TEXT_PRIMARY);
    
    // Draw checkbox items with 3D styling
    draw_checkbox_item(CHECKBOX_X, CHECKBOX1_Y, "FADER", g_options.send_fader, (g_options_selected_checkbox == 0));
    draw_checkbox_item(CHECKBOX_X, CHECKBOX2_Y, "EQUALIZER", g_options.send_eq, (g_options_selected_checkbox == 1));
    
    // Draw usage instructions at bottom with styled text
    draw_debug_text(&g_botScreen, "UP/DOWN: Select", WIN_X + 10, WIN_Y + 155, 0.35f, CLR_TEXT_SECONDARY);
    draw_debug_text(&g_botScreen, "A: Toggle  B: Exit", WIN_X + 10, WIN_Y + 165, 0.35f, CLR_TEXT_SECONDARY);
}

// ============================================================================
// INPUT HANDLING
// ============================================================================

void handle_options_input(u32 kDown)
{
    if (!g_options_window_open) return;
    
    // D-Pad navigation
    if (kDown & KEY_UP) {
        g_options_selected_checkbox = 0;
    }
    if (kDown & KEY_DOWN) {
        g_options_selected_checkbox = 1;
    }
    
    // A button: toggle selected option
    if (kDown & KEY_A) {
        if (g_options_selected_checkbox == 0) {
            g_options.send_fader = 1 - g_options.send_fader;
        } else {
            g_options.send_eq = 1 - g_options.send_eq;
        }
        save_options();
    }
    
    // B button: close window
    if (kDown & KEY_B) {
        g_options_window_open = 0;
    }
    
    // Touch input (only on NEW touch, not held)
    if (g_isTouched && !g_wasTouched) {
        float touch_x = g_touchPos.px;
        float touch_y = g_touchPos.py;
        
        // Check if touch is on checkbox 1
        if (touch_x >= CHECKBOX_X && touch_x < CHECKBOX_X + CHECKBOX_SIZE &&
            touch_y >= CHECKBOX1_Y && touch_y < CHECKBOX1_Y + CHECKBOX_SIZE) {
            g_options.send_fader = 1 - g_options.send_fader;
            g_options_selected_checkbox = 0;
            save_options();
        }
        
        // Check if touch is on checkbox 2
        if (touch_x >= CHECKBOX_X && touch_x < CHECKBOX_X + CHECKBOX_SIZE &&
            touch_y >= CHECKBOX2_Y && touch_y < CHECKBOX2_Y + CHECKBOX_SIZE) {
            g_options.send_eq = 1 - g_options.send_eq;
            g_options_selected_checkbox = 1;
            save_options();
        }
    }
}
