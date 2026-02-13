#include "common.h"
#include "options_window.h"
#include <unistd.h>

// Global options
Options g_options = {1, 1};  // Default: both enabled
int g_options_window_open = 0;
int g_options_selected_checkbox = 0;  // 0=fader, 1=eq

#define OPTIONS_FILE "/3ds/x18mixer/Options"

// Initialize options with defaults
void init_options(void)
{
    g_options.send_fader = 1;
    g_options.send_eq = 1;
    g_options_selected_checkbox = 0;
}

// Load options from file
void load_options(void)
{
    FILE *f = fopen(OPTIONS_FILE, "r");
    if (!f) {
        // File doesn't exist, use defaults
        init_options();
        return;
    }
    
    char line[256];
    char section[64] = {0};
    
    while (fgets(line, sizeof(line), f)) {
        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#' || line[0] == ';') continue;
        
        // Check for section header [SECTION]
        if (line[0] == '[') {
            int len = 0;
            for (int i = 1; line[i] != ']' && line[i] != '\0' && len < 63; i++) {
                section[len++] = line[i];
            }
            section[len] = '\0';
            continue;
        }
        
        // Parse key=value pairs within [OSC_SEND] section
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

// Save options to file
void save_options(void)
{
    FILE *f = fopen(OPTIONS_FILE, "w");
    if (!f) {
        return;
    }
    
    // Write header
    fprintf(f, "# X18 Mixer Options\n");
    fprintf(f, "# This file is auto-generated, do not edit manually\n\n");
    
    // Write [OSC_SEND] section
    fprintf(f, "[OSC_SEND]\n");
    fprintf(f, "fader=%d\n", g_options.send_fader);
    fprintf(f, "eq=%d\n", g_options.send_eq);
    
    fflush(f);
    fsync(fileno(f));
    fclose(f);
}

// Helper: Draw a single pixel character using rectangles (simple bitmap font)
static void draw_pixel_char(float x, float y, char c, u32 color, float pixel_size)
{
    // Simple bitmap font using pixel patterns
    // Each character is drawn as a pattern of small rectangles
    // pixel_size is the size of each "pixel" square
    
    switch(c) {
        case 'F':  // FADER: F
            // F: vertical bar on left, 2 horizontal bars
            C2D_DrawRectSolid(x, y, 0.55f, pixel_size, pixel_size*5, color);           // Left
            C2D_DrawRectSolid(x, y, 0.55f, pixel_size*3, pixel_size, color);           // Top
            C2D_DrawRectSolid(x, y + pixel_size*2, 0.55f, pixel_size*2, pixel_size, color);  // Middle
            break;
        case 'A':
            // A: two diagonal lines meeting at top, horizontal bar
            C2D_DrawRectSolid(x, y + pixel_size*2, 0.55f, pixel_size*4, pixel_size, color);  // Top diagonal
            C2D_DrawRectSolid(x, y, 0.55f, pixel_size, pixel_size*5, color);           // Left
            C2D_DrawRectSolid(x + pixel_size*3, y, 0.55f, pixel_size, pixel_size*5, color);  // Right
            C2D_DrawRectSolid(x, y + pixel_size*2, 0.55f, pixel_size*4, pixel_size, color);  // Middle
            break;
        case 'D':
            // D: vertical bar + curved right
            C2D_DrawRectSolid(x, y, 0.55f, pixel_size, pixel_size*5, color);           // Left
            C2D_DrawRectSolid(x + pixel_size, y, 0.55f, pixel_size*2, pixel_size, color);    // Top
            C2D_DrawRectSolid(x + pixel_size, y + pixel_size*4, 0.55f, pixel_size*2, pixel_size, color);  // Bottom
            C2D_DrawRectSolid(x + pixel_size*2, y + pixel_size, 0.55f, pixel_size, pixel_size*3, color);  // Right
            break;
        case 'E':
            // E: three horizontal bars + left vertical
            C2D_DrawRectSolid(x, y, 0.55f, pixel_size, pixel_size*5, color);           // Left
            C2D_DrawRectSolid(x, y, 0.55f, pixel_size*3, pixel_size, color);           // Top
            C2D_DrawRectSolid(x, y + pixel_size*2, 0.55f, pixel_size*2, pixel_size, color);  // Middle
            C2D_DrawRectSolid(x, y + pixel_size*4, 0.55f, pixel_size*3, pixel_size, color);  // Bottom
            break;
        case 'L':
            // L: vertical + bottom horizontal
            C2D_DrawRectSolid(x, y, 0.55f, pixel_size, pixel_size*5, color);           // Left
            C2D_DrawRectSolid(x, y + pixel_size*4, 0.55f, pixel_size*3, pixel_size, color);  // Bottom
            break;
        case 'Q':
            // Q: box + diagonal tail
            C2D_DrawRectSolid(x, y, 0.55f, pixel_size, pixel_size*4, color);           // Left
            C2D_DrawRectSolid(x + pixel_size*3, y, 0.55f, pixel_size, pixel_size*4, color);  // Right
            C2D_DrawRectSolid(x, y, 0.55f, pixel_size*4, pixel_size, color);           // Top
            C2D_DrawRectSolid(x, y + pixel_size*3, 0.55f, pixel_size*4, pixel_size, color);  // Bottom
            C2D_DrawRectSolid(x + pixel_size*2, y + pixel_size*3, 0.55f, pixel_size, pixel_size*2, color);  // Tail
            break;
        case 'R':
            // R: like P + diagonal leg
            C2D_DrawRectSolid(x, y, 0.55f, pixel_size, pixel_size*5, color);           // Left
            C2D_DrawRectSolid(x + pixel_size, y, 0.55f, pixel_size*2, pixel_size, color);    // Top
            C2D_DrawRectSolid(x + pixel_size*2, y + pixel_size, 0.55f, pixel_size, pixel_size*1, color);    // Right
            C2D_DrawRectSolid(x, y + pixel_size*2, 0.55f, pixel_size*2, pixel_size, color);  // Middle
            C2D_DrawRectSolid(x + pixel_size*2, y + pixel_size*3, 0.55f, pixel_size, pixel_size*2, color);  // Leg
            break;
        default:
            break;
    }
}

// Helper: Draw string using pixel characters
static void draw_pixel_string(float x, float y, const char *text, u32 color, float pixel_size)
{
    float current_x = x;
    for (int i = 0; text[i] != '\0'; i++) {
        draw_pixel_char(current_x, y, text[i], color, pixel_size);
        current_x += pixel_size * 5;  // Space between characters
    }
}

// Render options window on bottom screen
void render_options_window(void)
{
    if (!g_options_window_open) return;
    
    // NOTE: C2D_SceneBegin already called by render_show_manager - just draw on top
    // Create a centered dialog window (not full screen)
    
    float win_w = 240.0f;
    float win_h = 180.0f;
    float win_x = (SCREEN_WIDTH_BOT - win_w) / 2.0f;
    float win_y = (SCREEN_HEIGHT_BOT - win_h) / 2.0f;
    
    u32 clrWinBg = C2D_Color32(0x30, 0x30, 0x50, 0xFF);        // Dark blue background
    u32 clrWinBorder = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);    // White border
    u32 clrTitle = C2D_Color32(0x80, 0xFF, 0xFF, 0xFF);        // Cyan title
    u32 clrLabel = C2D_Color32(0xFF, 0xFF, 0x00, 0xFF);        // Yellow text for labels
    u32 clrCheckboxEnabled = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);  // Green for enabled/selected
    u32 clrCheckboxDisabled = C2D_Color32(0x44, 0x44, 0x88, 0xFF);  // Blue for disabled
    u32 clrCheckmark = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);    // Green checkmark
    
    // Draw window background
    C2D_DrawRectSolid(win_x, win_y, 0.51f, win_w, win_h, clrWinBg);
    
    // Draw window border (white)
    C2D_DrawRectSolid(win_x, win_y, 0.52f, win_w, 3, clrWinBorder);                      // Top
    C2D_DrawRectSolid(win_x, win_y + win_h - 3, 0.52f, win_w, 3, clrWinBorder);          // Bottom
    C2D_DrawRectSolid(win_x, win_y, 0.52f, 3, win_h, clrWinBorder);                      // Left
    C2D_DrawRectSolid(win_x + win_w - 3, win_y, 0.52f, 3, win_h, clrWinBorder);          // Right
    
    // Title
    draw_debug_text(&g_botScreen, "OSC Send Options", win_x + 15, win_y + 12, 0.55f, clrTitle);
    
    // ===== CHECKBOX 1: Send Fader =====
    float y1 = win_y + 50;
    float checkbox_x = win_x + 20;
    float checkbox_size = 22;
    
    // Draw background box for label 1 (BEHIND text, z-depth 0.40)
    float label_y1 = y1 - 20;
    C2D_DrawRectSolid(checkbox_x, label_y1, 0.40f, 45, 16, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
    C2D_DrawRectSolid(checkbox_x, label_y1, 0.41f, 45, 16, C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF)); // White border
    
    // Draw label "FADER" using pixel characters (bright yellow)
    draw_pixel_string(checkbox_x + 3, label_y1 + 2, "FADER", C2D_Color32(0xFF, 0xFF, 0x00, 0xFF), 2.0f);
    
    // Determine checkbox color based on state
    u32 box1_color = g_options.send_fader ? clrCheckboxEnabled : clrCheckboxDisabled;
    
    // Draw checkbox 1 (INFRONT of everything, z-depth 0.61)
    C2D_DrawRectSolid(checkbox_x, y1, 0.61f, checkbox_size, checkbox_size, box1_color);
    C2D_DrawRectSolid(checkbox_x, y1, 0.62f, checkbox_size, 2, clrWinBorder);   // Top border
    C2D_DrawRectSolid(checkbox_x, y1 + checkbox_size - 2, 0.62f, checkbox_size, 2, clrWinBorder); // Bottom border
    C2D_DrawRectSolid(checkbox_x, y1, 0.62f, 2, checkbox_size, clrWinBorder);   // Left border
    C2D_DrawRectSolid(checkbox_x + checkbox_size - 2, y1, 0.62f, 2, checkbox_size, clrWinBorder); // Right border
    
    // Draw checkmark/X inside checkbox (bigger and more visible)
    if (g_options.send_fader) {
        draw_debug_text(&g_botScreen, "✓", checkbox_x + 2, y1 + 1, 0.8f, clrCheckmark);
    } else {
        draw_debug_text(&g_botScreen, "✗", checkbox_x + 2, y1 + 1, 0.8f, C2D_Color32(0xFF, 0x00, 0x00, 0xFF));
    }
    
    // Add indicator bar below checkbox 1 if selected for navigation
    if (g_options_selected_checkbox == 0) {
        C2D_DrawRectSolid(checkbox_x, y1 + checkbox_size + 3, 0.62f, checkbox_size + 120, 2, clrTitle);
    }
    
    // ===== CHECKBOX 2: Send EQ =====
    float y2 = win_y + 105;
    
    // Draw background box for label 2 (BEHIND text, z-depth 0.40)
    float label_y2 = y2 - 20;
    C2D_DrawRectSolid(checkbox_x, label_y2, 0.40f, 60, 16, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
    C2D_DrawRectSolid(checkbox_x, label_y2, 0.41f, 60, 16, C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF)); // White border
    
    // Draw label "EQUALIZER" using pixel characters (bright yellow)
    draw_pixel_string(checkbox_x + 3, label_y2 + 2, "EQUALIZER", C2D_Color32(0xFF, 0xFF, 0x00, 0xFF), 1.5f);
    
    // Determine checkbox color based on state
    u32 box2_color = g_options.send_eq ? clrCheckboxEnabled : clrCheckboxDisabled;
    
    // Draw checkbox 2 (INFRONT of everything, z-depth 0.61)
    C2D_DrawRectSolid(checkbox_x, y2, 0.61f, checkbox_size, checkbox_size, box2_color);
    C2D_DrawRectSolid(checkbox_x, y2, 0.62f, checkbox_size, 2, clrWinBorder);   // Top border
    C2D_DrawRectSolid(checkbox_x, y2 + checkbox_size - 2, 0.62f, checkbox_size, 2, clrWinBorder); // Bottom border
    C2D_DrawRectSolid(checkbox_x, y2, 0.62f, 2, checkbox_size, clrWinBorder);   // Left border
    C2D_DrawRectSolid(checkbox_x + checkbox_size - 2, y2, 0.62f, 2, checkbox_size, clrWinBorder); // Right border
    
    // Draw checkmark/X inside checkbox (bigger and more visible)
    if (g_options.send_eq) {
        draw_debug_text(&g_botScreen, "✓", checkbox_x + 2, y2 + 1, 0.8f, clrCheckmark);
    } else {
        draw_debug_text(&g_botScreen, "✗", checkbox_x + 2, y2 + 1, 0.8f, C2D_Color32(0xFF, 0x00, 0x00, 0xFF));
    }
    
    // Add indicator bar below checkbox 2 if selected for navigation
    if (g_options_selected_checkbox == 1) {
        C2D_DrawRectSolid(checkbox_x, y2 + checkbox_size + 3, 0.62f, checkbox_size + 120, 2, clrTitle);
    }
}

// Handle input for options window
void handle_options_input(u32 kDown)
{
    if (!g_options_window_open) return;
    
    // Keyboard navigation
    if (kDown & KEY_UP) {
        g_options_selected_checkbox = 0;
    }
    if (kDown & KEY_DOWN) {
        g_options_selected_checkbox = 1;
    }
    
    // Toggle with A button
    if (kDown & KEY_A) {
        if (g_options_selected_checkbox == 0) {
            g_options.send_fader = 1 - g_options.send_fader;
        } else {
            g_options.send_eq = 1 - g_options.send_eq;
        }
        save_options();
    }
    
    // Close with B button
    if (kDown & KEY_B) {
        g_options_window_open = 0;
    }
    
    // Touch input detection - only on NEW touch (edge), not held
    int touch_edge = g_isTouched && !g_wasTouched;
    
    if (touch_edge) {
        float win_w = 240.0f;
        float win_h = 180.0f;
        float win_x = (SCREEN_WIDTH_BOT - win_w) / 2.0f;
        float win_y = (SCREEN_HEIGHT_BOT - win_h) / 2.0f;
        
        float checkbox_x = win_x + 20;
        float checkbox_size = 22;
        float y1 = win_y + 50;
        float y2 = win_y + 105;
        
        // Check if touch is on checkbox 1 (fader)
        if (g_touchPos.px >= checkbox_x && g_touchPos.px < checkbox_x + checkbox_size &&
            g_touchPos.py >= y1 && g_touchPos.py < y1 + checkbox_size) {
            g_options.send_fader = 1 - g_options.send_fader;
            save_options();
            g_options_selected_checkbox = 0;
        }
        
        // Check if touch is on checkbox 2 (eq)
        if (g_touchPos.px >= checkbox_x && g_touchPos.px < checkbox_x + checkbox_size &&
            g_touchPos.py >= y2 && g_touchPos.py < y2 + checkbox_size) {
            g_options.send_eq = 1 - g_options.send_eq;
            save_options();
            g_options_selected_checkbox = 1;
        }
    }
}
