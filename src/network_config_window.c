#include "common.h"
#include "network_config_window.h"

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
    draw_debug_text(&g_topScreen, "Network Config", win_x + 20, win_y + 20, 0.5f, clrLabel);
    
    // IP label and display
    float field_y = win_y + 60.0f;
    draw_debug_text(&g_topScreen, "Mixer IP:", win_x + 20, field_y, 0.4f, clrLabel);
    
    float ip_input_x = win_x + 120.0f;
    float input_w = SCREEN_WIDTH_TOP - 160.0f;
    float input_h = 20.0f;
    
    u32 ip_bg_color = (g_net_selected_field == 0) ? clrSelectedField : clrInputBg;
    u32 ip_border_color = (g_net_selected_field == 0) ? clrSelectedField : C2D_Color32(0x60, 0x60, 0x80, 0xFF);
    C2D_DrawRectSolid(ip_input_x, field_y - 2, 0.46f, input_w, input_h, ip_bg_color);
    C2D_DrawRectangle(ip_input_x, field_y - 2, 0.46f, input_w, input_h, ip_border_color, ip_border_color, ip_border_color, ip_border_color);
    
    // Format and display IP address with selected digit in red
    char ip_display[20];
    ip_digits_to_display(g_net_ip_digits, ip_display, sizeof(ip_display));
    
    // Draw IP address character by character, highlighting the selected digit in red
    if (g_net_selected_field == 0) {
        // Draw IP with selected digit highlighted
        int digit_idx = 0;
        int digit_x_count = 0;  // Count only non-dot characters for x position
        for (int i = 0; ip_display[i] != '\0'; i++) {
            char ch = ip_display[i];
            u32 color = clrText;
            float scale = 0.32f;
            
            // Only count non-dot characters as digit positions
            if (ch != '.') {
                // Check if this is the selected digit position
                if (digit_idx == g_net_digit_index) {
                    color = clrSelectedDigit;  // Red for selected
                    scale = 0.5f;  // Bold for selected
                }
                digit_idx++;
            }
            
            // Draw the character (larger and bold if selected)
            // Use digit_x_count for position to avoid offset from dots
            char char_str[2] = {ch, '\0'};
            draw_debug_text(&g_topScreen, char_str, ip_input_x + 4 + digit_x_count * 9, field_y - 1, scale, color);
            
            // Only increment x position counter for non-dot characters
            if (ch != '.') {
                digit_x_count++;
            }
        }
    } else {
        u32 ip_text_color = clrText;
        draw_debug_text(&g_topScreen, ip_display, ip_input_x + 4, field_y - 1, 0.32f, ip_text_color);
    }
    
    // Port label and display
    float port_y = field_y + 50.0f;
    draw_debug_text(&g_topScreen, "Port:", win_x + 20, port_y, 0.4f, clrLabel);
    
    u32 port_bg_color = (g_net_selected_field == 1) ? clrSelectedField : clrInputBg;
    u32 port_border_color = (g_net_selected_field == 1) ? clrSelectedField : C2D_Color32(0x60, 0x60, 0x80, 0xFF);
    C2D_DrawRectSolid(ip_input_x, port_y - 2, 0.46f, input_w, input_h, port_bg_color);
    C2D_DrawRectangle(ip_input_x, port_y - 2, 0.46f, input_w, input_h, port_border_color, port_border_color, port_border_color, port_border_color);
    
    // Draw port with selected digit highlighted in red
    if (g_net_selected_field == 1) {
        int char_idx = 0;
        for (int i = 0; g_net_port_input[i] != '\0'; i++) {
            char ch = g_net_port_input[i];
            u32 color = (i == g_net_digit_index) ? clrSelectedDigit : clrText;
            float scale = (i == g_net_digit_index) ? 0.5f : 0.32f;
            
            char char_str[2] = {ch, '\0'};
            draw_debug_text(&g_topScreen, char_str, ip_input_x + 4 + char_idx * 9, port_y - 1, scale, color);
            char_idx++;
        }
    } else {
        draw_debug_text(&g_topScreen, g_net_port_input, ip_input_x + 4, port_y - 1, 0.32f, clrText);
    }
    
    // Instructions at the bottom
    draw_debug_text(&g_topScreen, "D-Pad: LEFT/RIGHT=Move  UP/DOWN=Change digit  A=Save  B=Cancel", win_x + 20, port_y + 50, 0.3f, clrLabel);
    
    // Restore bottom screen context
    C2D_SceneBegin(g_botScreen.target);
}

// Handle input for network configuration window
void handle_net_config_input(u32 kDown)
{
    // B button closes without saving
    if (kDown & KEY_B) {
        g_net_config_open = 0;
        g_net_digit_index = 0;
        return;
    }
    
    // A button saves configuration
    if (kDown & KEY_A) {
        save_network_config();
        g_net_config_open = 0;
        g_net_digit_index = 0;
        return;
    }
    
    // D-Pad Left/Right to move between digit positions
    if (kDown & KEY_DLEFT) {
        if (g_net_digit_index > 0) {
            g_net_digit_index--;
        } else {
            // Wrap around to previous field
            if (g_net_selected_field == 0) {
                // Go to port's last digit
                g_net_selected_field = 1;
                int port_len = strlen(g_net_port_input);
                g_net_digit_index = (port_len > 0) ? port_len - 1 : 0;
            } else {
                // Go to IP's last digit (always 11 for 12 digits)
                g_net_selected_field = 0;
                g_net_digit_index = 11;
            }
        }
        return;
    }
    
    if (kDown & KEY_DRIGHT) {
        int max_ip_digits = 12;  // Always 12 digits for IP
        int max_port_digits = strlen(g_net_port_input);
        
        if (g_net_selected_field == 0) {
            // IP field
            if (g_net_digit_index < max_ip_digits - 1) {
                g_net_digit_index++;
            } else {
                // Switch to port field
                g_net_selected_field = 1;
                g_net_digit_index = 0;
            }
        } else {
            // Port field
            if (g_net_digit_index < max_port_digits - 1) {
                g_net_digit_index++;
            } else {
                // Wrap back to IP field
                g_net_selected_field = 0;
                g_net_digit_index = 0;
            }
        }
        return;
    }
    
    // D-Pad Up/Down to change digit value (0-9 with wrap)
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

// You'll need to implement save_network_config() and load_network_config()
// in the main file or another appropriate module
