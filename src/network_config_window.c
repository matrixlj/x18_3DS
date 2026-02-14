#include "common.h"
#include "network_config_window.h"

// Touch debounce tracking
static int g_net_touch_was_active = 0;
int is_valid_ip(const char *ip)
{
    if (!ip || strlen(ip) == 0) return 0;
    
    int octects[4] = {0};
    int count = sscanf(ip, "%d.%d.%d.%d", &octects[0], &octects[1], &octects[2], &octects[3]);
    
    if (count != 4) return 0;
    
    for (int i = 0; i < 4; i++) {
        if (octects[i] < 0 || octects[i] > 255) return 0;
    }
    
    return 1;
}

// Load IP and Port from net.txt file on window open
void load_network_config_from_file(void)
{
    FILE *f = fopen("/3ds/x18_mixer/net.txt", "r");
    if (!f) {
        // File doesn't exist, leave fields empty
        g_net_ip_input[0] = '\0';
        g_net_port_input[0] = '\0';
        return;
    }
    
    // Read IP (first line)
    if (fgets(g_net_ip_input, sizeof(g_net_ip_input), f)) {
        // Remove newline
        size_t len = strlen(g_net_ip_input);
        if (len > 0 && g_net_ip_input[len - 1] == '\n') {
            g_net_ip_input[len - 1] = '\0';
        }
    } else {
        g_net_ip_input[0] = '\0';
    }
    
    // Read Port (second line)
    if (fgets(g_net_port_input, sizeof(g_net_port_input), f)) {
        // Remove newline
        size_t len = strlen(g_net_port_input);
        if (len > 0 && g_net_port_input[len - 1] == '\n') {
            g_net_port_input[len - 1] = '\0';
        }
    } else {
        g_net_port_input[0] = '\0';
    }
    
    fclose(f);
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
    u32 clrOctectBg = C2D_Color32(0xDD, 0xDD, 0xDD, 0xFF);      // Light gray background
    u32 clrOctectText = C2D_Color32(0x00, 0x00, 0x00, 0xFF);    // Black text
    u32 clrOctectBorder = C2D_Color32(0x60, 0x60, 0x60, 0xFF);  // Dark gray border
    
    C2D_DrawRectSolid(win_x, win_y, 0.40f, win_w, win_h, clrWinBg);
    C2D_DrawRectangle(win_x, win_y, 0.40f, win_w, win_h, clrWinBorder, clrWinBorder, clrWinBorder, clrWinBorder);
    
    // Title
    draw_debug_text(&g_topScreen, "Network Config", win_x + 20, win_y + 15, 0.5f, clrLabel);
    
    // IP field
    float field_y = win_y + 70.0f;
    draw_debug_text(&g_topScreen, "Mixer IP:", win_x + 20, field_y, 0.5f, clrLabel);
    
    float input_x = win_x + 130.0f;
    float input_w = 230.0f;
    float input_h = 30.0f;
    
    u32 ip_bg = (g_net_selected_field == 0) ? C2D_Color32(0xFF, 0xFF, 0x00, 0xFF) : clrOctectBg;
    u32 ip_border = (g_net_selected_field == 0) ? C2D_Color32(0xFF, 0x00, 0x00, 0xFF) : clrOctectBorder;
    
    C2D_DrawRectSolid(input_x, field_y - 2, 0.48f, input_w, input_h, ip_bg);
    C2D_DrawRectangle(input_x, field_y - 2, 0.48f, input_w, input_h, ip_border, ip_border, ip_border, ip_border);
    
    draw_debug_text(&g_topScreen, g_net_ip_input, input_x + 5, field_y, 0.50f, clrOctectText);
    
    // Port field
    float port_y = field_y + 50.0f;
    draw_debug_text(&g_topScreen, "Port:", win_x + 20, port_y, 0.5f, clrLabel);
    
    u32 port_bg = (g_net_selected_field == 1) ? C2D_Color32(0xFF, 0xFF, 0x00, 0xFF) : clrOctectBg;
    u32 port_border = (g_net_selected_field == 1) ? C2D_Color32(0xFF, 0x00, 0x00, 0xFF) : clrOctectBorder;
    
    C2D_DrawRectSolid(input_x, port_y - 2, 0.48f, 100.0f, input_h, port_bg);
    C2D_DrawRectangle(input_x, port_y - 2, 0.48f, 100.0f, input_h, port_border, port_border, port_border, port_border);
    
    draw_debug_text(&g_topScreen, g_net_port_input, input_x + 5, port_y, 0.50f, clrOctectText);
    
    // Validation status (only for IP)
    u32 status_color = is_valid_ip(g_net_ip_input) ? C2D_Color32(0x00, 0xFF, 0x00, 0xFF) : C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
    const char *status_text = is_valid_ip(g_net_ip_input) ? "Valid IP" : "Invalid IP";
    draw_debug_text(&g_topScreen, status_text, win_x + 20, port_y + 40, 0.50f, status_color);
    
    // Instructions
    draw_debug_text(&g_topScreen, "D-Pad: Navigate  A: Press key  B: Cancel", win_x + 20, port_y + 60, 0.45f, clrLabel);
    
    // BOTTOM SCREEN ==========================================================
    C2D_SceneBegin(g_botScreen.target);
    
    // Clear bottom screen with dark background
    C2D_DrawRectSolid(0.0f, 0.0f, 0.5f, SCREEN_WIDTH_BOT, SCREEN_HEIGHT_BOT, C2D_Color32(0x10, 0x10, 0x20, 0xFF));
    
    // Display current field being edited
    float kbd_y = 10.0f;
    u32 clrKbdLabel = C2D_Color32(0x80, 0xFF, 0xFF, 0xFF);
    
    const char *field_label = (g_net_selected_field == 0) ? "IP Address:" : "Port:";
    draw_debug_text(&g_botScreen, field_label, 10, kbd_y, 0.55f, clrKbdLabel);
    
    // Display current value
    float value_y = kbd_y + 25.0f;
    const char *current_val = (g_net_selected_field == 0) ? g_net_ip_input : g_net_port_input;
    draw_debug_text(&g_botScreen, current_val, 10, value_y, 0.70f, C2D_Color32(0xFF, 0xFF, 0x00, 0xFF));
    
    // Virtual keyboard: 
    // Row 1: 1 2 3 4 5
    // Row 2: 6 7 8 9 0
    // Row 3: . DEL TAB SAVE
    
    float keypad_x = 5.0f;
    float keypad_y = value_y + 20.0f;  // Much higher, more accessible
    float key_w = 48.0f;
    float key_h = 40.0f;
    float key_spacing = 50.0f;
    
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
    
    // Point button (key 10)
    int is_dot_selected = (g_net_keyboard_selected == 10);
    draw_keyboard_button(".", keypad_x, row3_y, key_w, key_h, is_dot_selected);
    
    // DEL button (key 11)
    int is_del_selected = (g_net_keyboard_selected == 11);
    draw_keyboard_button("DEL", keypad_x + key_spacing, row3_y, key_w, key_h, is_del_selected);
    
    // TAB button to switch field (key 12)
    int is_tab_selected = (g_net_keyboard_selected == 12);
    draw_keyboard_button("TAB", keypad_x + key_spacing * 2, row3_y, key_w, key_h, is_tab_selected);
    
    // SAVE button (key 13)
    int is_save_selected = (g_net_keyboard_selected == 13);
    draw_keyboard_button("SAVE", keypad_x + key_spacing * 3, row3_y, key_w, key_h, is_save_selected);
    
    // Instructions
    float instr_y = row3_y + key_spacing + 5.0f;
    draw_debug_text(&g_botScreen, "D-Pad: Navigate  A: Press key  B: Cancel", 10, instr_y, 0.45f, clrKbdLabel);
}

// Handle input for network configuration window
void handle_net_config_input(u32 kDown)
{
    // B button closes
    if (kDown & KEY_B) {
        g_net_config_open = 0;
        return;
    }
    
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
                g_net_keyboard_selected = 10 + (g_net_keyboard_selected - 5);
            }
        } else if (g_net_keyboard_selected < 13) {
            g_net_keyboard_selected++;
        }
        return;
    }
    
    // A button presses the selected key - detect KEY_A press transition
    if (kDown & KEY_A) {
        // This fires when KEY_A transitions from not-pressed to pressed
        // So we ignore repeated presses naturally through the button press/release cycle
        
        char *current_field = (g_net_selected_field == 0) ? g_net_ip_input : g_net_port_input;
        int max_len = (g_net_selected_field == 0) ? 63 : 5;
        
        if (g_net_keyboard_selected < 10) {
            // Digit pressed (0-9)
            // Keyboard layout: 1 2 3 4 5 / 6 7 8 9 0
            // So key 0-4 = digit 1-5, key 5-8 = digit 6-9, key 9 = digit 0
            int digit = (g_net_keyboard_selected == 9) ? 0 : (g_net_keyboard_selected + 1);
            
            char digit_ch = '0' + digit;
            if (strlen(current_field) < max_len) {
                int len = strlen(current_field);
                current_field[len] = digit_ch;
                current_field[len + 1] = '\0';
            }
        } else if (g_net_keyboard_selected == 10) {
            // Point pressed (only for IP field)
            if (g_net_selected_field == 0 && strlen(current_field) < max_len) {
                int len = strlen(current_field);
                current_field[len] = '.';
                current_field[len + 1] = '\0';
            }
        } else if (g_net_keyboard_selected == 11) {
            // DEL pressed
            int len = strlen(current_field);
            if (len > 0) {
                current_field[len - 1] = '\0';
            }
        } else if (g_net_keyboard_selected == 12) {
            // TAB pressed: switch to next field
            if (g_net_selected_field == 0) {
                g_net_selected_field = 1;
                g_net_keyboard_selected = 0;  // Reset keyboard position
            } else {
                g_net_selected_field = 0;
                g_net_keyboard_selected = 0;
            }
        } else if (g_net_keyboard_selected == 13) {
            // SAVE pressed: save and close
            save_network_config();
            g_net_config_open = 0;
        }
        return;
    }
}

// Handle touch input for network configuration
void handle_net_config_touch(touchPosition touch)
{
    if (!g_net_config_open) return;
    
    // Touch disabled for now - use D-Pad navigation instead
    return;
    
    // Touch debounce: only process on touch edge (when touch state changes)
    int touch_is_active = g_isTouched;
    int touch_edge = touch_is_active && !g_net_touch_was_active;
    g_net_touch_was_active = touch_is_active;
    
    if (!touch_edge) return;  // Ignore if not a new touch
    
    // Top screen field selection (py < 120)
    if (touch.py < 120) {
        float input_x = 130.0f;
        float input_w = 230.0f;
        float field_y = 68.0f;
        float input_h = 30.0f;
        
        // IP field
        if (touch.px >= input_x && touch.px < input_x + input_w &&
            touch.py >= field_y && touch.py < field_y + input_h) {
            g_net_selected_field = 0;
            return;
        }
        
        // Port field
        float port_y = field_y + 50.0f;
        if (touch.px >= input_x && touch.px < input_x + 100.0f &&
            touch.py >= port_y && touch.py < port_y + input_h) {
            g_net_selected_field = 1;
            return;
        }
    }
    
    // Bottom screen keyboard (py >= 120)
    // Map global coordinates to local keyboard area
    // Keyboard layout: keypad_x=5, keypad_y=55 (in bottom screen local coords)
    // These are rendering coordinates - use them directly
    float py_local = touch.py - 120.0f;  // Convert to bottom screen local
    
    // Skip if not in keyboard area
    if (py_local < 55.0f || py_local >= 195.0f) return;
    if (touch.px < 5.0f || touch.px >= 250.0f) return;
    
    // Use same layout as rendering
    float keypad_x = 5.0f;
    float keypad_y = 55.0f;  // Row 1 starts here
    float key_w = 48.0f;
    float key_h = 40.0f;
    float key_spacing = 50.0f;
    
    // Row 1: digits 1-5 (y: 55-95)
    if (py_local >= keypad_y && py_local < keypad_y + key_h) {
        for (int col = 0; col < 5; col++) {
            float key_x = keypad_x + col * key_spacing;
            if (touch.px >= key_x && touch.px < key_x + key_w) {
                g_net_keyboard_selected = col;  // Key 0-4 = digit 1-5
                handle_net_config_input(KEY_A);
                return;
            }
        }
    }
    
    // Row 2: digits 6-0 (y: 105-145)
    float row2_y = keypad_y + key_spacing;
    if (py_local >= row2_y && py_local < row2_y + key_h) {
        for (int col = 0; col < 5; col++) {
            float key_x = keypad_x + col * key_spacing;
            if (touch.px >= key_x && touch.px < key_x + key_w) {
                g_net_keyboard_selected = 5 + col;  // Key 5-9 = digit 6-0
                handle_net_config_input(KEY_A);
                return;
            }
        }
    }
    
    // Row 3: ., DEL, TAB, SAVE (y: 155-195)
    float row3_y = row2_y + key_spacing;
    if (py_local >= row3_y && py_local < row3_y + key_h) {
        for (int col = 0; col < 4; col++) {
            float key_x = keypad_x + col * key_spacing;
            if (touch.px >= key_x && touch.px < key_x + key_w) {
                g_net_keyboard_selected = 10 + col;  // Key 10-13
                handle_net_config_input(KEY_A);
                return;
            }
        }
    }
}
