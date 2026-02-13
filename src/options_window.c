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

// Render options window on bottom screen
void render_options_window(void)
{
    if (!g_options_window_open) return;
    
    // NOTE: C2D_SceneBegin already called by render_show_manager - just draw on top
    // Create a centered dialog window (not full screen)
    
    float win_w = 220.0f;
    float win_h = 160.0f;
    float win_x = (SCREEN_WIDTH_BOT - win_w) / 2.0f;
    float win_y = (SCREEN_HEIGHT_BOT - win_h) / 2.0f;
    
    u32 clrWinBg = C2D_Color32(0x30, 0x30, 0x50, 0xFF);        // Dark blue background
    u32 clrWinBorder = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);    // White border
    u32 clrTitle = C2D_Color32(0x80, 0xFF, 0xFF, 0xFF);        // Cyan title
    u32 clrText = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);         // White text
    u32 clrCheckboxBg = C2D_Color32(0x15, 0x15, 0x25, 0xFF);   // Very dark blue for unchecked
    u32 clrCheckboxSelected = C2D_Color32(0x00, 0x88, 0xFF, 0xFF);  // Light blue for selected
    u32 clrCheckmark = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);    // Green checkmark
    
    // Draw window background
    C2D_DrawRectSolid(win_x, win_y, 0.51f, win_w, win_h, clrWinBg);
    
    // Draw window border (white)
    C2D_DrawRectSolid(win_x, win_y, 0.52f, win_w, 2, clrWinBorder);                      // Top
    C2D_DrawRectSolid(win_x, win_y + win_h - 2, 0.52f, win_w, 2, clrWinBorder);          // Bottom
    C2D_DrawRectSolid(win_x, win_y, 0.52f, 2, win_h, clrWinBorder);                      // Left
    C2D_DrawRectSolid(win_x + win_w - 2, win_y, 0.52f, 2, win_h, clrWinBorder);          // Right
    
    // Title
    draw_debug_text(&g_botScreen, "OSC Options", win_x + 10, win_y + 10, 0.5f, clrTitle);
    
    // Checkbox 1: Send Fader
    float checkbox_y1 = win_y + 40;
    float checkbox_x = win_x + 15;
    float checkbox_size = 24;
    u32 checkbox1_color = (g_options_selected_checkbox == 0) ? clrCheckboxSelected : clrCheckboxBg;
    
    // Draw checkbox 1 background
    C2D_DrawRectSolid(checkbox_x, checkbox_y1, 0.61f, checkbox_size, checkbox_size, checkbox1_color);
    // Draw checkbox 1 border
    C2D_DrawRectSolid(checkbox_x, checkbox_y1, 0.62f, checkbox_size, 2, clrWinBorder);   // Top
    C2D_DrawRectSolid(checkbox_x, checkbox_y1 + checkbox_size - 2, 0.62f, checkbox_size, 2, clrWinBorder); // Bottom
    C2D_DrawRectSolid(checkbox_x, checkbox_y1, 0.62f, 2, checkbox_size, clrWinBorder);   // Left
    C2D_DrawRectSolid(checkbox_x + checkbox_size - 2, checkbox_y1, 0.62f, 2, checkbox_size, clrWinBorder); // Right
    
    // Draw checkmark if enabled
    if (g_options.send_fader) {
        draw_debug_text(&g_botScreen, "✓", checkbox_x + 4, checkbox_y1 + 2, 0.55f, clrCheckmark);
    }
    
    // Label for checkbox 1
    draw_debug_text(&g_botScreen, "Send Fader", checkbox_x + 32, checkbox_y1 + 5, 0.55f, clrText);
    
    // Checkbox 2: Send EQ
    float checkbox_y2 = win_y + 90;
    u32 checkbox2_color = (g_options_selected_checkbox == 1) ? clrCheckboxSelected : clrCheckboxBg;
    
    // Draw checkbox 2 background
    C2D_DrawRectSolid(checkbox_x, checkbox_y2, 0.61f, checkbox_size, checkbox_size, checkbox2_color);
    // Draw checkbox 2 border
    C2D_DrawRectSolid(checkbox_x, checkbox_y2, 0.62f, checkbox_size, 2, clrWinBorder);   // Top
    C2D_DrawRectSolid(checkbox_x, checkbox_y2 + checkbox_size - 2, 0.62f, checkbox_size, 2, clrWinBorder); // Bottom
    C2D_DrawRectSolid(checkbox_x, checkbox_y2, 0.62f, 2, checkbox_size, clrWinBorder);   // Left
    C2D_DrawRectSolid(checkbox_x + checkbox_size - 2, checkbox_y2, 0.62f, 2, checkbox_size, clrWinBorder); // Right
    
    // Draw checkmark if enabled
    if (g_options.send_eq) {
        draw_debug_text(&g_botScreen, "✓", checkbox_x + 4, checkbox_y2 + 2, 0.55f, clrCheckmark);
    }
    
    // Label for checkbox 2
    draw_debug_text(&g_botScreen, "Send Equalizer", checkbox_x + 32, checkbox_y2 + 5, 0.55f, clrText);
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
    
    // Touch input detection
    if (g_isTouched) {
        float win_w = 220.0f;
        float win_h = 160.0f;
        float win_x = (SCREEN_WIDTH_BOT - win_w) / 2.0f;
        float win_y = (SCREEN_HEIGHT_BOT - win_h) / 2.0f;
        
        float checkbox_x = win_x + 15;
        float checkbox_y1 = win_y + 40;
        float checkbox_y2 = win_y + 90;
        float checkbox_size = 24;
        
        // Check if touch is on checkbox 1 (fader)
        if (g_touchPos.px >= checkbox_x && g_touchPos.px < checkbox_x + checkbox_size &&
            g_touchPos.py >= checkbox_y1 && g_touchPos.py < checkbox_y1 + checkbox_size) {
            g_options.send_fader = 1 - g_options.send_fader;
            save_options();
            g_options_selected_checkbox = 0;
        }
        
        // Check if touch is on checkbox 2 (eq)
        if (g_touchPos.px >= checkbox_x && g_touchPos.px < checkbox_x + checkbox_size &&
            g_touchPos.py >= checkbox_y2 && g_touchPos.py < checkbox_y2 + checkbox_size) {
            g_options.send_eq = 1 - g_options.send_eq;
            save_options();
            g_options_selected_checkbox = 1;
        }
    }
}
