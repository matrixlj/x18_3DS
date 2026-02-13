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
    
    C2D_SceneBegin(g_botScreen.target);
    
    // Background
    u32 clrBg = C2D_Color32(0x20, 0x20, 0x40, 0xFF);
    u32 clrBorder = C2D_Color32(0x80, 0x80, 0xFF, 0xFF);
    u32 clrLabel = C2D_Color32(0x80, 0xFF, 0xFF, 0xFF);
    u32 clrText = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    u32 clrCheckboxBg = C2D_Color32(0x15, 0x15, 0x25, 0xFF);
    u32 clrCheckboxSelected = C2D_Color32(0x00, 0x00, 0xFF, 0xFF);
    u32 clrCheckmark = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);
    
    C2D_DrawRectSolid(0, 0, 0.5f, SCREEN_WIDTH_BOT, SCREEN_HEIGHT_BOT, clrBg);
    C2D_DrawRectangle(0, 0, 0.5f, SCREEN_WIDTH_BOT, SCREEN_HEIGHT_BOT, clrBorder, clrBorder, clrBorder, clrBorder);
    
    // Title
    draw_debug_text(&g_botScreen, "OSC Send Options", 10, 20, 0.5f, clrLabel);
    
    // Checkbox 1: Fader
    float checkbox_y1 = 70;
    float checkbox_x = 30;
    u32 checkbox1_color = (g_options_selected_checkbox == 0) ? clrCheckboxSelected : clrCheckboxBg;
    
    C2D_DrawRectSolid(checkbox_x, checkbox_y1, 0.51f, 16, 16, checkbox1_color);
    C2D_DrawRectangle(checkbox_x, checkbox_y1, 0.51f, 16, 16, clrBorder, clrBorder, clrBorder, clrBorder);
    
    if (g_options.send_fader) {
        draw_debug_text(&g_botScreen, "✓", checkbox_x + 3, checkbox_y1 + 2, 0.4f, clrCheckmark);
    }
    
    draw_debug_text(&g_botScreen, "Send Fader", checkbox_x + 30, checkbox_y1 + 2, 0.35f, clrText);
    
    // Checkbox 2: EQ
    float checkbox_y2 = 130;
    u32 checkbox2_color = (g_options_selected_checkbox == 1) ? clrCheckboxSelected : clrCheckboxBg;
    
    C2D_DrawRectSolid(checkbox_x, checkbox_y2, 0.51f, 16, 16, checkbox2_color);
    C2D_DrawRectangle(checkbox_x, checkbox_y2, 0.51f, 16, 16, clrBorder, clrBorder, clrBorder, clrBorder);
    
    if (g_options.send_eq) {
        draw_debug_text(&g_botScreen, "✓", checkbox_x + 3, checkbox_y2 + 2, 0.4f, clrCheckmark);
    }
    
    draw_debug_text(&g_botScreen, "Send Equalizer", checkbox_x + 30, checkbox_y2 + 2, 0.35f, clrText);
    
    // Help text
    draw_debug_text(&g_botScreen, "Up/Down: Navigate", 20, 200, 0.3f, clrLabel);
    draw_debug_text(&g_botScreen, "A: Toggle  B: Close", 20, 215, 0.3f, clrLabel);
}

// Handle input for options window
void handle_options_input(u32 kDown)
{
    if (!g_options_window_open) return;
    
    if (kDown & KEY_UP) {
        g_options_selected_checkbox = 0;
    }
    if (kDown & KEY_DOWN) {
        g_options_selected_checkbox = 1;
    }
    
    if (kDown & KEY_A) {
        // Toggle selected checkbox
        if (g_options_selected_checkbox == 0) {
            g_options.send_fader = 1 - g_options.send_fader;
        } else {
            g_options.send_eq = 1 - g_options.send_eq;
        }
        save_options();
    }
    
    if (kDown & KEY_B) {
        g_options_window_open = 0;
    }
}
