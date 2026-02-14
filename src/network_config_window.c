#include "common.h"
#include "network_config_window.h"

// Preset configurations
#define LOCALHOST_IP "000000000001"
#define LOCALHOST_PORT "1024"
#define LOCAL_IP "192168001001"
#define LOCAL_PORT "1024"

// Helper function to format IP address from digits (xxxxx -> xxx.xxx.xxx.xxx)
void ip_digits_to_display(const char *digits, char *display_buf, int max_len)
{
    int len = strlen(digits);
    // Pad to 12 digits
    char padded[13] = {0};
    int pad_count = 12 - len;
    for (int i = 0; i < pad_count; i++) padded[i] = '0';
    strcpy(padded + pad_count, digits);
    
    // Format as xxx.xxx.xxx.xxx
    snprintf(display_buf, max_len, "%c%c%c.%c%c%c.%c%c%c.%c%c%c",
             padded[0], padded[1], padded[2],
             padded[3], padded[4], padded[5],
             padded[6], padded[7], padded[8],
             padded[9], padded[10], padded[11]);
}

// Draw a 3D preset button
void draw_preset_button(C2D_Screen *screen, const char *label, float x, float y, float w, float h, int is_selected)
{
    u32 clrLight, clrDark, clrText;
    
    if (is_selected) {
        clrLight = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);  // Bright green
        clrDark = C2D_Color32(0x00, 0x88, 0x00, 0xFF);   // Dark green
        clrText = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);   // White text
    } else {
        clrLight = C2D_Color32(0xAA, 0xAA, 0xAA, 0xFF);  // Light gray
        clrDark = C2D_Color32(0x55, 0x55, 0x55, 0xFF);   // Dark gray
        clrText = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);   // White text
    }
    
    // 3D effect: top-left light, bottom-right dark
    C2D_DrawRectSolid(x, y, 0.48f, w / 2.0f, h / 2.0f, clrLight);
    u32 clrMed = C2D_Color32(
        ((clrLight >> 0) & 0xFF) / 2 + ((clrDark >> 0) & 0xFF) / 2,
        ((clrLight >> 8) & 0xFF) / 2 + ((clrDark >> 8) & 0xFF) / 2,
        ((clrLight >> 16) & 0xFF) / 2 + ((clrDark >> 16) & 0xFF) / 2,
        0xFF
    );
    C2D_DrawRectSolid(x + w / 2.0f, y, 0.48f, w / 2.0f, h / 2.0f, clrMed);
    C2D_DrawRectSolid(x, y + h / 2.0f, 0.48f, w / 2.0f, h / 2.0f, clrMed);
    C2D_DrawRectSolid(x + w / 2.0f, y + h / 2.0f, 0.48f, w / 2.0f, h / 2.0f, clrDark);
    
    // Border
    u32 clrBorder = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    C2D_DrawRectangle(x, y, 0.48f, w, h, clrBorder, clrBorder, clrBorder, clrBorder);
    
    // Label
    draw_debug_text(screen, label, x + 5, y + h / 2.0f - 5, 0.35f, clrText);
}

// Draw a virtual keyboard button on bottom screen
void draw_keyboard_button(const char *label, float x, float y, float w, float h, int is_selected)
{
    u32 clrBg, clrBorder, clrText;
    
    if (is_selected) {
        clrBg = C2D_Color32(0xFF, 0xAA, 0x00, 0xFF);     // Orange when selected
        clrBorder = C2D_Color32(0xFF, 0xFF, 0x00, 0xFF); // Yellow border
        clrText = C2D_Color32(0x00, 0x00, 0x00, 0xFF);   // Black text
    } else {
        clrBg = C2D_Color32(0x70, 0x70, 0x70, 0xFF);     // Dark gray
        clrBorder = C2D_Color32(0xAA, 0xAA, 0xAA, 0xFF); // Light gray border
        clrText = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);   // White text
    }
    
    C2D_DrawRectSolid(x, y, 0.5f, w, h, clrBg);
    C2D_DrawRectangle(x, y, 0.5f, w, h, clrBorder, clrBorder, clrBorder, clrBorder);
    
    // Draw label centered
    draw_debug_text(&g_botScreen, label, x + w/2.0f - 5, y + h/2.0f - 5, 0.35f, clrText);
}

// Render network configuration window
void render_net_config_window(void)
{
    if (!g_net_config_open) return;
    
    // TOP SCREEN ============================================================
    C2D_SceneBegin(g_topScreen.target);
    
    float win_x = 0.0f;
    float win_y = 0.0f;
    float win_w = SCREEN_WIDTH_TOP;
    float win_h = SCREEN_HEIGHT_TOP;
    
    u32 clrWinBg = C2D_Color32(0x20, 0x20, 0x40, 0xFF);
    u32 clrWinBorder = C2D_Color32(0x80, 0x80, 0xFF, 0xFF);
    u32 clrLabel = C2D_Color32(0x80, 0xFF, 0xFF, 0xFF);
    
    C2D_DrawRectSolid(win_x, win_y, 0.40f, win_w, win_h, clrWinBg);
    C2D_DrawRectangle(win_x, win_y, 0.40f, win_w, win_h, clrWinBorder, clrWinBorder, clrWinBorder, clrWinBorder);
    
    // Title
    draw_debug_text(&g_topScreen, "Network Config", win_x + 20, win_y + 15, 0.5f, clrLabel);
    
    if (g_net_edit_mode == 0) {
        // ========== PRESET SELECTION MODE ==========
        draw_debug_text(&g_topScreen, "Select Preset:", win_x + 20, win_y + 60, 0.4f, clrLabel);
        
        float button_y = win_y + 90.0f;
        float button_w = 100.0f;
        float button_h = 35.0f;
        float button_spacing = 110.0f;
        
        // Localhost button
        draw_preset_button(&g_topScreen, "Localhost", win_x + 30, button_y, button_w, button_h, (g_net_preset_mode == 0));
        
        // Local button
        draw_preset_button(&g_topScreen, "Local", win_x + 30 + button_spacing, button_y, button_w, button_h, (g_net_preset_mode == 1));
        
        // Manual button
        draw_preset_button(&g_topScreen, "Manual", win_x + 30 + button_spacing * 2, button_y, button_w, button_h, (g_net_preset_mode == 2));
        
        // Info for selected preset
        float info_y = button_y + button_h + 20.0f;
        if (g_net_preset_mode >= 0) {
            char info_buf[100];
            if (g_net_preset_mode == 0) {
                snprintf(info_buf, sizeof(info_buf), "Localhost: 127.0.0.1:1024");
            } else if (g_net_preset_mode == 1) {
                snprintf(info_buf, sizeof(info_buf), "Local: 192.168.1.1:1024");
            } else {
                snprintf(info_buf, sizeof(info_buf), "Manual: Enter custom IP");
            }
            draw_debug_text(&g_topScreen, info_buf, win_x + 20, info_y, 0.35f, clrLabel);
        }
        
        // Instructions
        draw_debug_text(&g_topScreen, "LEFT/RIGHT: Navigate  A: Select  B: Cancel", win_x + 20, win_y + 210, 0.3f, clrLabel);
        
    } else {
        // ========== MANUAL EDIT MODE ==========
        float field_y = win_y + 60.0f;
        draw_debug_text(&g_topScreen, "Mixer IP:", win_x + 20, field_y, 0.4f, clrLabel);
        
        // Draw IP with clear, dark text on light background
        float ip_start_x = win_x + 130.0f;
        float octect_w = 50.0f;
        float octect_h = 25.0f;
        float octect_spacing = 55.0f;
        
        char padded_ip[13] = {0};
        int len = strlen(g_net_ip_digits);
        int pad_count = 12 - len;
        for (int i = 0; i < pad_count; i++) padded_ip[i] = '0';
        strcpy(padded_ip + pad_count, g_net_ip_digits);
        
        // Extract octects
        int o1, o2, o3, o4;
        char oct_str[4] = {0};
        
        strncpy(oct_str, padded_ip + 0, 3);
        o1 = atoi(oct_str);
        memset(oct_str, 0, sizeof(oct_str));
        strncpy(oct_str, padded_ip + 3, 3);
        o2 = atoi(oct_str);
        memset(oct_str, 0, sizeof(oct_str));
        strncpy(oct_str, padded_ip + 6, 3);
        o3 = atoi(oct_str);
        memset(oct_str, 0, sizeof(oct_str));
        strncpy(oct_str, padded_ip + 9, 3);
        o4 = atoi(oct_str);
        
        // Draw octect boxes with LIGHT background and DARK text for readability
        u32 clrOctectBg = C2D_Color32(0xDD, 0xDD, 0xDD, 0xFF);  // Light gray background
        u32 clrOctectText = C2D_Color32(0x00, 0x00, 0x00, 0xFF); // Black text
        u32 clrOctectBorder = C2D_Color32(0x60, 0x60, 0x60, 0xFF); // Dark gray border
        
        for (int i = 0; i < 4; i++) {
            float oct_x = ip_start_x + i * octect_spacing;
            
            C2D_DrawRectSolid(oct_x, field_y - 2, 0.48f, octect_w, octect_h, clrOctectBg);
            C2D_DrawRectangle(oct_x, field_y - 2, 0.48f, octect_w, octect_h, clrOctectBorder, clrOctectBorder, clrOctectBorder, clrOctectBorder);
            
            // Draw octect value in dark text
            char oct_val[4];
            int oct_num = (i == 0) ? o1 : (i == 1) ? o2 : (i == 2) ? o3 : o4;
            snprintf(oct_val, sizeof(oct_val), "%d", oct_num);
            draw_debug_text(&g_topScreen, oct_val, oct_x + 10, field_y, 0.4f, clrOctectText);
        }
        
        // Separator dots
        for (int i = 0; i < 3; i++) {
            float dot_x = ip_start_x + (i + 1) * octect_spacing - 15.0f;
            draw_debug_text(&g_topScreen, ".", dot_x, field_y, 0.4f, clrOctectText);
        }
        
        // Port field
        float port_y = field_y + 50.0f;
        draw_debug_text(&g_topScreen, "Port:", win_x + 20, port_y, 0.4f, clrLabel);
        
        C2D_DrawRectSolid(ip_start_x, port_y - 2, 0.48f, 100.0f, octect_h, clrOctectBg);
        C2D_DrawRectangle(ip_start_x, port_y - 2, 0.48f, 100.0f, octect_h, clrOctectBorder, clrOctectBorder, clrOctectBorder, clrOctectBorder);
        
        draw_debug_text(&g_topScreen, g_net_port_input, ip_start_x + 10, port_y, 0.35f, clrOctectText);
        
        // Validation status
        int o1_valid = (o1 >= 0 && o1 <= 255) ? 1 : 0;
        int o2_valid = (o2 >= 0 && o2 <= 255) ? 1 : 0;
        int o3_valid = (o3 >= 0 && o3 <= 255) ? 1 : 0;
        int o4_valid = (o4 >= 0 && o4 <= 255) ? 1 : 0;
        int all_valid = o1_valid && o2_valid && o3_valid && o4_valid;
        
        u32 status_color = all_valid ? C2D_Color32(0x00, 0xFF, 0x00, 0xFF) : C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
        const char *status_text = all_valid ? "Valid IP" : "Invalid IP";
        draw_debug_text(&g_topScreen, status_text, win_x + 20, port_y + 40, 0.35f, status_color);
        
        // Instructions for edit mode
        draw_debug_text(&g_topScreen, "Use numeric keypad on bottom screen", win_x + 20, port_y + 60, 0.3f, clrLabel);
        draw_debug_text(&g_topScreen, "Note: Type full octects (0-255)", win_x + 20, port_y + 75, 0.3f, clrLabel);
    }
    
    // BOTTOM SCREEN ==========================================================
    C2D_SceneBegin(g_botScreen.target);
    
    // Clear bottom screen with dark background
    C2D_DrawRectSolid(0.0f, 0.0f, 0.5f, SCREEN_WIDTH_BOT, SCREEN_HEIGHT_BOT, C2D_Color32(0x10, 0x10, 0x20, 0xFF));
    
    if (g_net_edit_mode == 1) {
        // ========== VIRTUAL KEYBOARD ==========
        
        // Display current field being edited
        float kbd_y = 10.0f;
        u32 clrKbdLabel = C2D_Color32(0x80, 0xFF, 0xFF, 0xFF);
        const char *field_name = (g_net_selected_field == 0) ? "IP Address" : "Port";
        draw_debug_text(&g_botScreen, field_name, 10, kbd_y, 0.35f, clrKbdLabel);
        
        // Display current input value
        float value_y = kbd_y + 20.0f;
        char display_val[64];
        if (g_net_selected_field == 0) {
            char ip_display[20];
            ip_digits_to_display(g_net_ip_digits, ip_display, sizeof(ip_display));
            snprintf(display_val, sizeof(display_val), "IP: %s", ip_display);
        } else {
            snprintf(display_val, sizeof(display_val), "Port: %s", g_net_port_input);
        }
        draw_debug_text(&g_botScreen, display_val, 10, value_y, 0.3f, C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF));
        
        // Virtual keyboard: 3 rows
        // Row 1: 1 2 3 4 5
        // Row 2: 6 7 8 9 0
        // Row 3: DEL OK BACK SAVE
        
        float keypad_x = 20.0f;
        float keypad_y = value_y + 40.0f;
        float key_w = 40.0f;
        float key_h = 35.0f;
        float key_spacing = 45.0f;
        
        // Row 1: digits 1-5
        for (int i = 1; i <= 5; i++) {
            char digit_str[2] = {'0' + i, '\0'};
            int is_selected = (g_net_keyboard_selected == (i - 1));
            draw_keyboard_button(digit_str, keypad_x + (i - 1) * key_spacing, keypad_y, key_w, key_h, is_selected);
        }
        
        // Row 2: digits 6-0 (6-9, then 0)
        float row2_y = keypad_y + key_spacing;
        for (int i = 6; i <= 10; i++) {
            char digit_str[2] = {'0' + (i == 10 ? 0 : i), '\0'};
            int is_selected = (g_net_keyboard_selected == (i - 1));
            draw_keyboard_button(digit_str, keypad_x + (i - 6) * key_spacing, row2_y, key_w, key_h, is_selected);
        }
        
        // Row 3: special buttons
        float row3_y = row2_y + key_spacing;
        
        // DEL button (key 10)
        int is_del_selected = (g_net_keyboard_selected == 10);
        draw_keyboard_button("DEL", keypad_x, row3_y, key_w, key_h, is_del_selected);
        
        // OK button (key 11)
        int is_ok_selected = (g_net_keyboard_selected == 11);
        draw_keyboard_button("OK", keypad_x + key_spacing, row3_y, key_w, key_h, is_ok_selected);
        
        // BACK button to switch field or presets (key 12)
        int is_back_selected = (g_net_keyboard_selected == 12);
        draw_keyboard_button("BACK", keypad_x + key_spacing * 2, row3_y, key_w, key_h, is_back_selected);
        
        // SAVE button (key 13)
        int is_save_selected = (g_net_keyboard_selected == 13);
        draw_keyboard_button("SAVE", keypad_x + key_spacing * 3, row3_y, key_w, key_h, is_save_selected);
        
        // Instructions
        float instr_y = row3_y + key_spacing + 5.0f;
        draw_debug_text(&g_botScreen, "D-Pad: UP/DOWN/LEFT/RIGHT to navigate, A to select", 10, instr_y, 0.25f, clrKbdLabel);
    }
}

// Handle input for network configuration window
void handle_net_config_input(u32 kDown)
{
    // B button in preset mode closes, in edit mode goes back to presets
    if (kDown & KEY_B) {
        if (g_net_edit_mode == 1) {
            // Back to preset selection
            g_net_edit_mode = 0;
            g_net_keyboard_selected = -1;
        } else {
            // Close entirely
            g_net_config_open = 0;
            g_net_preset_mode = -1;
            g_net_edit_mode = 0;
            g_net_keyboard_selected = -1;
        }
        return;
    }
    
    if (g_net_edit_mode == 0) {
        // ========== PRESET SELECTION MODE ==========
        
        // LEFT/RIGHT to navigate presets
        if (kDown & KEY_DLEFT) {
            if (g_net_preset_mode > 0) {
                g_net_preset_mode--;
            } else {
                g_net_preset_mode = 2;  // Wrap to Manual
            }
            return;
        }
        
        if (kDown & KEY_DRIGHT) {
            if (g_net_preset_mode < 2) {
                g_net_preset_mode++;
            } else {
                g_net_preset_mode = 0;  // Wrap to Localhost
            }
            return;
        }
        
        // A button selects preset
        if (kDown & KEY_A) {
            if (g_net_preset_mode == 0) {
                // Localhost
                strcpy(g_net_ip_digits, LOCALHOST_IP);
                strcpy(g_net_port_input, LOCALHOST_PORT);
            } else if (g_net_preset_mode == 1) {
                // Local
                strcpy(g_net_ip_digits, LOCAL_IP);
                strcpy(g_net_port_input, LOCAL_PORT);
            } else if (g_net_preset_mode == 2) {
                // Manual - enter edit mode
                g_net_edit_mode = 1;
                g_net_selected_field = 0;
                g_net_keyboard_selected = 0;  // Start on first key
                return;
            }
            
            // For presets 0 and 1, save immediately
            if (g_net_preset_mode < 2) {
                save_network_config();
                g_net_config_open = 0;
                g_net_preset_mode = -1;
            }
            return;
        }
        
    } else {
        // ========== MANUAL EDIT MODE WITH VIRTUAL KEYBOARD ==========
        
        // D-Pad to navigate keyboard
        if (kDown & KEY_DLEFT) {
            if (g_net_keyboard_selected > 0) {
                g_net_keyboard_selected--;
            }
            return;
        }
        
        if (kDown & KEY_DRIGHT) {
            if (g_net_keyboard_selected < 13) {
                g_net_keyboard_selected++;
            }
            return;
        }
        
        if (kDown & KEY_DUP) {
            // Move up one row
            if (g_net_keyboard_selected >= 5) {
                g_net_keyboard_selected -= 5;
            }
            return;
        }
        
        if (kDown & KEY_DDOWN) {
            // Move down one row
            if (g_net_keyboard_selected < 10) {
                if (g_net_keyboard_selected < 5) {
                    g_net_keyboard_selected += 5;
                } else {
                    g_net_keyboard_selected = 10 + (g_net_keyboard_selected - 5);  // Move to special buttons
                }
            } else if (g_net_keyboard_selected < 13) {
                g_net_keyboard_selected++;
            }
            return;
        }
        
        // A button presses the selected key
        if (kDown & KEY_A) {
            char *current_field = (g_net_selected_field == 0) ? g_net_ip_digits : g_net_port_input;
            int max_len = (g_net_selected_field == 0) ? 12 : 5;
            
            if (g_net_keyboard_selected < 10) {
                // Digit pressed (0-9)
                char digit = '0' + g_net_keyboard_selected;
                if (strlen(current_field) < max_len - 1 || (strlen(current_field) == 1 && current_field[0] == '0')) {
                    if (strlen(current_field) == 1 && current_field[0] == '0') {
                        // Replace leading zero
                        current_field[0] = digit;
                    } else {
                        // Append digit
                        strncat(current_field, &digit, 1);
                    }
                }
            } else if (g_net_keyboard_selected == 10) {
                // DEL pressed
                int len = strlen(current_field);
                if (len > 1) {
                    current_field[len - 1] = '\0';
                } else if (len == 1) {
                    current_field[0] = '0';
                }
            } else if (g_net_keyboard_selected == 11) {
                // OK pressed: switch to next field
                if (g_net_selected_field == 0) {
                    g_net_selected_field = 1;
                } else {
                    g_net_selected_field = 0;
                }
            } else if (g_net_keyboard_selected == 12) {
                // BACK pressed: go back to presets
                g_net_edit_mode = 0;
                g_net_keyboard_selected = -1;
            } else if (g_net_keyboard_selected == 13) {
                // SAVE pressed: save and close
                save_network_config();
                g_net_config_open = 0;
                g_net_keyboard_selected = -1;
                g_net_preset_mode = -1;
                g_net_edit_mode = 0;
            }
            return;
        }
    }
}
