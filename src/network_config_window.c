#include "common.h"
#include "network_config_window.h"

// Preset configurations
#define LOCALHOST_IP "000000000001"
#define LOCALHOST_PORT "1024"
#define LOCAL_IP "192168001001"
#define LOCAL_PORT "1024"

// Helper function to format IP address from digits (xxxxx -> xxx.xxx.xxx.xxx)
// Pads with zeros on left if needed: "101099112" -> "000.101.099.112"
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

// Validate if octect is valid (0-255)
int is_valid_octect(const char *octect_str) {
    if (!octect_str || strlen(octect_str) == 0) return 0;
    int val = atoi(octect_str);
    return val >= 0 && val <= 255;
}

// Extract octects from IP digit string
void get_octects_from_digits(const char *digits, int *o1, int *o2, int *o3, int *o4) {
    char padded[13] = {0};
    int len = strlen(digits);
    int pad_count = 12 - len;
    for (int i = 0; i < pad_count; i++) padded[i] = '0';
    strcpy(padded + pad_count, digits);
    
    char oct_str[4] = {0};
    strncpy(oct_str, padded + 0, 3);
    *o1 = atoi(oct_str);
    
    memset(oct_str, 0, sizeof(oct_str));
    strncpy(oct_str, padded + 3, 3);
    *o2 = atoi(oct_str);
    
    memset(oct_str, 0, sizeof(oct_str));
    strncpy(oct_str, padded + 6, 3);
    *o3 = atoi(oct_str);
    
    memset(oct_str, 0, sizeof(oct_str));
    strncpy(oct_str, padded + 9, 3);
    *o4 = atoi(oct_str);
}

// Draw a 3D preset button (similar to EQ buttons)
void draw_preset_button(C2D_Screen *screen, const char *label, float x, float y, float w, float h, int is_selected, u32 clrBase)
{
    u32 clrDark, clrLight, clrText;
    
    if (is_selected) {
        // Selected: bright green
        clrLight = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);  // Bright green
        clrDark = C2D_Color32(0x00, 0x88, 0x00, 0xFF);   // Dark green
        clrText = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);   // White text
    } else {
        // Not selected: gray
        clrLight = C2D_Color32(0xAA, 0xAA, 0xAA, 0xFF);  // Light gray
        clrDark = C2D_Color32(0x55, 0x55, 0x55, 0xFF);   // Dark gray
        clrText = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);   // White text
    }
    
    // Top-left to middle (light)
    C2D_DrawRectSolid(x, y, 0.48f, w / 2.0f, h / 2.0f, clrLight);
    // Top-right to middle (medium)
    u32 clrMed = C2D_Color32(
        ((clrLight >> 0) & 0xFF) / 2 + ((clrDark >> 0) & 0xFF) / 2,
        ((clrLight >> 8) & 0xFF) / 2 + ((clrDark >> 8) & 0xFF) / 2,
        ((clrLight >> 16) & 0xFF) / 2 + ((clrDark >> 16) & 0xFF) / 2,
        0xFF
    );
    C2D_DrawRectSolid(x + w / 2.0f, y, 0.48f, w / 2.0f, h / 2.0f, clrMed);
    // Bottom-left to middle (medium)
    C2D_DrawRectSolid(x, y + h / 2.0f, 0.48f, w / 2.0f, h / 2.0f, clrMed);
    // Bottom-right to middle (dark)
    C2D_DrawRectSolid(x + w / 2.0f, y + h / 2.0f, 0.48f, w / 2.0f, h / 2.0f, clrDark);
    
    // Border
    u32 clrBorder = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    C2D_DrawRectangle(x, y, 0.48f, w, h, clrBorder, clrBorder, clrBorder, clrBorder);
    
    // Label
    draw_debug_text(screen, label, x + 5, y + h / 2.0f - 5, 0.35f, clrText);
}

// Render network configuration window (on top screen)
void render_net_config_window(void)
{
    if (!g_net_config_open) return;
    
    // Switch to top screen context
    C2D_SceneBegin(g_topScreen.target);
    
    // Full screen on top screen
    float win_x = 0.0f;
    float win_y = 0.0f;
    float win_w = SCREEN_WIDTH_TOP;
    float win_h = SCREEN_HEIGHT_TOP;
    
    u32 clrWinBg = C2D_Color32(0x20, 0x20, 0x40, 0xFF);
    u32 clrWinBorder = C2D_Color32(0x80, 0x80, 0xFF, 0xFF);
    u32 clrText = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    u32 clrLabel = C2D_Color32(0x80, 0xFF, 0xFF, 0xFF);
    u32 clrInputBg = C2D_Color32(0x15, 0x15, 0x25, 0xFF);
    u32 clrSelectedField = C2D_Color32(0x00, 0x00, 0xFF, 0xFF);  // Blue for selected field
    u32 clrSelectedDigit = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);  // Red for selected digit
    
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
        draw_preset_button(&g_topScreen, "Localhost", win_x + 30, button_y, button_w, button_h, (g_net_preset_mode == 0), C2D_Color32(0x00, 0x88, 0x00, 0xFF));
        
        // Local button
        draw_preset_button(&g_topScreen, "Local", win_x + 30 + button_spacing, button_y, button_w, button_h, (g_net_preset_mode == 1), C2D_Color32(0x00, 0x88, 0x00, 0xFF));
        
        // Manual button
        draw_preset_button(&g_topScreen, "Manual", win_x + 30 + button_spacing * 2, button_y, button_w, button_h, (g_net_preset_mode == 2), C2D_Color32(0x00, 0x88, 0x00, 0xFF));
        
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
        
        // Draw IP address with octects separated visually
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
        
        // Octect 1
        strncpy(oct_str, padded_ip + 0, 3);
        o1 = atoi(oct_str);
        
        // Octect 2
        memset(oct_str, 0, sizeof(oct_str));
        strncpy(oct_str, padded_ip + 3, 3);
        o2 = atoi(oct_str);
        
        // Octect 3
        memset(oct_str, 0, sizeof(oct_str));
        strncpy(oct_str, padded_ip + 6, 3);
        o3 = atoi(oct_str);
        
        // Octect 4
        memset(oct_str, 0, sizeof(oct_str));
        strncpy(oct_str, padded_ip + 9, 3);
        o4 = atoi(oct_str);
        
        // Draw octect boxes
        for (int i = 0; i < 4; i++) {
            float oct_x = ip_start_x + i * octect_spacing;
            u32 oct_bg = clrInputBg;
            
            // Highlight selected octect in edit mode if we're on IP field
            if (g_net_selected_field == 0) {
                int selected_octect = g_net_digit_index / 3;
                if (i == selected_octect) {
                    oct_bg = clrSelectedField;
                }
            }
            
            C2D_DrawRectSolid(oct_x, field_y - 2, 0.48f, octect_w, octect_h, oct_bg);
            C2D_DrawRectangle(oct_x, field_y - 2, 0.48f, octect_w, octect_h, clrLabel, clrLabel, clrLabel, clrLabel);
            
            // Draw octect value
            char oct_val[4];
            int oct_num = (i == 0) ? o1 : (i == 1) ? o2 : (i == 2) ? o3 : o4;
            snprintf(oct_val, sizeof(oct_val), "%d", oct_num);
            draw_debug_text(&g_topScreen, oct_val, oct_x + 10, field_y, 0.4f, clrText);
        }
        
        // Separator dots
        for (int i = 0; i < 3; i++) {
            float dot_x = ip_start_x + (i + 1) * octect_spacing - 15.0f;
            draw_debug_text(&g_topScreen, ".", dot_x, field_y, 0.4f, clrText);
        }
        
        // Port field
        float port_y = field_y + 50.0f;
        draw_debug_text(&g_topScreen, "Port:", win_x + 20, port_y, 0.4f, clrLabel);
        
        u32 port_bg_color = (g_net_selected_field == 1) ? clrSelectedField : clrInputBg;
        u32 port_border_color = (g_net_selected_field == 1) ? clrSelectedField : clrLabel;
        C2D_DrawRectSolid(ip_start_x, port_y - 2, 0.48f, 100.0f, octect_h, port_bg_color);
        C2D_DrawRectangle(ip_start_x, port_y - 2, 0.48f, 100.0f, octect_h, port_border_color, port_border_color, port_border_color, port_border_color);
        
        draw_debug_text(&g_topScreen, g_net_port_input, ip_start_x + 10, port_y, 0.35f, clrText);
        
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
        draw_debug_text(&g_topScreen, "LEFT/RIGHT: Move octect  UP/DOWN: Change digit", win_x + 20, port_y + 60, 0.3f, clrLabel);
        draw_debug_text(&g_topScreen, "A: Save  B: Back to presets", win_x + 20, port_y + 75, 0.3f, clrLabel);
    }
    
    // Restore bottom screen context
    C2D_SceneBegin(g_botScreen.target);
}

// Handle input for network configuration window
void handle_net_config_input(u32 kDown)
{
    // B button in preset mode closes, in edit mode goes back to presets
    if (kDown & KEY_B) {
        if (g_net_edit_mode == 1) {
            // Back to preset selection
            g_net_edit_mode = 0;
        } else {
            // Close entirely
            g_net_config_open = 0;
            g_net_preset_mode = -1;
            g_net_edit_mode = 0;
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
                g_net_digit_index = 0;
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
        // ========== MANUAL EDIT MODE ==========
        
        // A button saves configuration
        if (kDown & KEY_A) {
            save_network_config();
            g_net_config_open = 0;
            g_net_digit_index = 0;
            g_net_preset_mode = -1;
            g_net_edit_mode = 0;
            return;
        }
        
        // D-Pad Left/Right to move between digit positions (now octect-based)
        if (kDown & KEY_DLEFT) {
            if (g_net_selected_field == 0) {
                int current_octect = g_net_digit_index / 3;
                if (current_octect > 0) {
                    g_net_digit_index -= 3;
                } else {
                    // Wrap to port field (simplified - assume port has 4 digits)
                    g_net_selected_field = 1;
                    g_net_digit_index = strlen(g_net_port_input) - 1;
                }
            } else {
                // Port field - single field, can't move left
                if (g_net_digit_index > 0) {
                    g_net_digit_index--;
                }
            }
            return;
        }
        
        if (kDown & KEY_DRIGHT) {
            if (g_net_selected_field == 0) {
                int current_octect = g_net_digit_index / 3;
                if (current_octect < 3) {
                    g_net_digit_index += 3;
                } else {
                    // Move to port field
                    g_net_selected_field = 1;
                    g_net_digit_index = 0;
                }
            } else {
                // Port field
                int port_len = strlen(g_net_port_input);
                if (g_net_digit_index < port_len - 1) {
                    g_net_digit_index++;
                }
            }
            return;
        }
        
        // D-Pad Up/Down to change digit value (with octect validation)
        if (kDown & KEY_DUP) {
            char *field = (g_net_selected_field == 0) ? g_net_ip_digits : g_net_port_input;
            int digit_val = field[g_net_digit_index] - '0';
            digit_val = (digit_val + 1) % 10;
            field[g_net_digit_index] = '0' + digit_val;
            return;
        }
        
        if (kDown & KEY_DDOWN) {
            char *field = (g_net_selected_field == 0) ? g_net_ip_digits : g_net_port_input;
            int digit_val = field[g_net_digit_index] - '0';
            digit_val = (digit_val - 1 + 10) % 10;
            field[g_net_digit_index] = '0' + digit_val;
            return;
        }
    }
}

// You'll need to implement save_network_config() and load_network_config()
// in the main file or another appropriate module
