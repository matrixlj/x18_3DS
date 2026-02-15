#include "common.h"
#include "show_manager_window.h"
#include "network_config_window.h"
#include "options_window.h"

// Color for DELETE/EXIT buttons  
#define CLR_X C2D_Color32(0xFF, 0x00, 0x00, 0xFF)

// Text colors for contrast
#define CLR_TEXT_DARK C2D_Color32(0x00, 0x00, 0x00, 0xFF)     // Black for bright buttons
#define CLR_TEXT_LIGHT C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF)    // White for dark buttons

// Forward declarations from main.c (external functions)
extern int check_button_touch(int button_idx);
extern int get_show_item_from_touch(void);
extern int load_show_from_file(const char *name, Show *show);
extern int delete_show_file(const char *name);
extern int duplicate_show_file(const char *src, const char *dst);
extern int rename_show_file(const char *old, const char *new);
extern void save_show_to_file(Show *show);
extern void list_available_shows(void);
extern void apply_step_to_faders(int step_idx);

void render_show_manager(void)
{
    C2D_TargetClear(g_botScreen.target, CLR_BG_PRIMARY);
    C2D_SceneBegin(g_botScreen.target);
    C2D_DrawRectSolid(0.0f, 0.0f, 0.5f, SCREEN_WIDTH_BOT, SCREEN_HEIGHT_BOT, CLR_BG_PRIMARY);
    
    // ===== HEADER (full width) =====
    draw_panel_header(0.0f, 0.0f, SCREEN_WIDTH_BOT, 32.0f, "Show Manager", CLR_BORDER_CYAN);
    
    // ===== LEFT COLUMN: Show List (0-205px wide) =====
    float list_x = 5.0f;
    float list_y = 40.0f;
    float list_w = 205.0f;
    float list_item_h = 16.0f;
    
    // List background panel with border
    C2D_DrawRectSolid(list_x, list_y, 0.50f, list_w, 195.0f, CLR_BG_SECONDARY);
    draw_3d_border(list_x, list_y, list_w, 195.0f, CLR_BORDER_BRIGHT, CLR_SHADOW_BLACK, 1);
    
    // Draw show items (compact list)
    u32 clrModified = CLR_BORDER_YELLOW;
    for (int i = 0; i < g_num_available_shows && i < 12; i++) {
        float item_y = list_y + 2.0f + (i * list_item_h);
        
        // Highlight selected item (keep depth high when Options is closed, or skip if open)
        if (i == g_selected_show && !g_options_window_open) {
            C2D_DrawRectSolid(list_x + 1, item_y, 0.60f, list_w - 2, list_item_h - 1, C2D_Color32(0x00, 0x44, 0x88, 0xFF));
        }
        
        u32 text_color = (i == g_selected_show) ? CLR_BORDER_CYAN : CLR_TEXT_SECONDARY;
        
        // Check if modified
        if (g_show_modified && strcmp(g_available_shows[i], g_current_show.name) == 0) {
            text_color = clrModified;
        }
        
        // Renaming mode
        if (g_renaming && i == g_selected_show) {
            draw_debug_text(&g_botScreen, g_new_name, list_x + 8, item_y + 1, 0.30f, CLR_BORDER_CYAN);
            draw_debug_text(&g_botScreen, "_", list_x + 8 + g_rename_input_pos * 6, item_y + 1, 0.30f, CLR_BORDER_CYAN);
        } else {
            // Show name (truncate if too long)
            char display_name[32];
            strncpy(display_name, g_available_shows[i], 28);
            display_name[28] = '\0';
            draw_debug_text(&g_botScreen, display_name, list_x + 8, item_y + 1, 0.30f, text_color);
        }
    }
    
    // ===== RIGHT COLUMN: Buttons (210-320px wide) =====
    float btn_x = 210.0f;
    float btn_w = 107.0f;
    float btn_h = 16.0f;
    float btn_y = 40.0f;
    
    // Button definitions: label, color, starts at y position
    struct {
        const char *label;
        u32 color;
        float y;
    } buttons[] = {
        {"LOAD", CLR_BORDER_GREEN, btn_y},
        {"DELETE", CLR_X, btn_y + 18},
        {"DUPLICATE", CLR_BORDER_ORANGE, btn_y + 36},
        {"RENAME", CLR_BORDER_YELLOW, btn_y + 54},
        {"", C2D_Color32(0x33, 0x33, 0x33, 0xFF), btn_y + 72},  // Spacer
        {"OPTIONS", CLR_BORDER_ORANGE, btn_y + 90},
        {"NETWORK", CLR_BORDER_CYAN, btn_y + 108},
        {"EXIT", CLR_X, btn_y + 126},
    };
    
    for (int i = 0; i < 8; i++) {
        if (i == 4) continue;  // Skip spacer
        
        draw_button_3d(btn_x + 2, buttons[i].y, btn_w - 4, btn_h, buttons[i].color);
        
        // Determine text color for contrast
        u32 text_color = CLR_TEXT_LIGHT;  // Default: white for dark buttons
        if (buttons[i].color == CLR_BORDER_GREEN || 
            buttons[i].color == CLR_BORDER_YELLOW ||
            buttons[i].color == CLR_BORDER_ORANGE ||
            buttons[i].color == CLR_BORDER_CYAN) {
            text_color = CLR_TEXT_DARK;  // Black for bright buttons
        }
        
        // Center text horizontally and vertically
        float button_center_x = btn_x + 2 + (btn_w - 4) / 2.0f;
        float button_center_y = buttons[i].y + btn_h / 2.0f;
        
        // Estimate text width (approximately 7.5px per character at 0.28f scale)
        int label_len = strlen(buttons[i].label);
        float approx_text_width = label_len * 7.5f;
        
        // Position text centered in button
        float text_x = button_center_x - approx_text_width / 2.0f;
        float text_y = button_center_y - 4.0f;  // Adjust Y for vertical centering
        
        draw_debug_text(&g_botScreen, buttons[i].label, text_x, text_y, 0.40f, text_color);
    }
    
    // Info panel on the right at the bottom
    float info_y = btn_y + 145;
    draw_3d_border(btn_x + 2, info_y, btn_w - 4, 85, CLR_BORDER_BRIGHT, CLR_SHADOW_BLACK, 1);
    C2D_DrawRectSolid(btn_x + 3, info_y + 1, 0.51f, btn_w - 6, 83, CLR_BG_SECONDARY);
    
    draw_debug_text(&g_botScreen, "Info:", btn_x + 8, info_y + 5, 0.32f, CLR_BORDER_CYAN);
    
    if (g_selected_show >= 0 && g_selected_show < g_num_available_shows) {
        // Show name
        char name_short[24];
        strncpy(name_short, g_available_shows[g_selected_show], 20);
        name_short[20] = '\0';
        draw_debug_text(&g_botScreen, "Show:", btn_x + 8, info_y + 20, 0.28f, CLR_TEXT_SECONDARY);
        draw_debug_text(&g_botScreen, name_short, btn_x + 8, info_y + 30, 0.25f, CLR_TEXT_PRIMARY);
        
        // Steps count (if loaded)
        if (strcmp(g_available_shows[g_selected_show], g_current_show.name) == 0) {
            char steps_str[16];
            snprintf(steps_str, sizeof(steps_str), "Steps: %d", g_current_show.num_steps);
            draw_debug_text(&g_botScreen, steps_str, btn_x + 8, info_y + 45, 0.25f, CLR_BORDER_YELLOW);
        }
    }
}

void handle_manager_input(void)
{
    u32 kDown = hidKeysDown();
    int touch_edge = g_isTouched && !g_wasTouched;
    
    // If network config window is open, handle its input
    if (g_net_config_open) {
        handle_net_config_input(kDown);
        if (g_isTouched) {
            handle_net_config_touch(g_touchPos);
        }
        return;  // Touch is disabled while window is open
    }
    
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
                    snprintf(new_name, sizeof(new_name), "%.57s_copy", g_available_shows[g_selected_show]);
                    duplicate_show_file(g_available_shows[g_selected_show], new_name);
                }
            } else if (check_button_touch(BTN_RENAME)) {
                if (g_selected_show >= 0 && g_selected_show < g_num_available_shows) {
                    strcpy(g_new_name, g_available_shows[g_selected_show]);
                    g_rename_input_pos = strlen(g_new_name);
                    g_renaming = 1;
                }
            } else if (check_button_touch(4)) {  // Button 4 = OPT
                // OPT button - open options window
                g_options_window_open = 1;
                g_options_selected_checkbox = 0;
            } else if (check_button_touch(5)) {  // Button 5 = NET
                // NET button - open network configuration window
                g_net_config_open = 1;
                g_net_selected_field = 0;
                g_net_keyboard_selected = 0;    // Start on first keyboard key
                load_network_config_from_file();  // Load from file if exists
            } else if (check_button_touch(6)) {  // Button 6 = EXIT
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

int check_button_touch(int button_idx)
{
    if (!g_isTouched) return 0;
    
    // Updated button layout: Vertical buttons on the right (210-320px)
    // LOAD(0), DELETE(1), DUP(2), REN(3), OPT(4), NET(5), EXIT(6)
    float btn_x = 210.0f;
    float btn_w = 107.0f;
    float btn_h = 16.0f;
    float btn_y_start = 40.0f;
    
    float btn_positions[] = {
        btn_y_start + 0,      // LOAD (0)
        btn_y_start + 18,     // DELETE (1)
        btn_y_start + 36,     // DUP (2)
        btn_y_start + 54,     // REN (3)
        btn_y_start + 90,     // OPT (4)
        btn_y_start + 108,    // NET (5)
        btn_y_start + 126,    // EXIT (6)
    };
    
    if (button_idx >= 0 && button_idx <= 6) {
        float btn_y = btn_positions[button_idx];
        return (g_touchPos.px >= btn_x && g_touchPos.px < btn_x + btn_w &&
                g_touchPos.py >= btn_y && g_touchPos.py < btn_y + btn_h);
    }
    
    return 0;
}

int get_show_item_from_touch(void)
{
    if (!g_isTouched) return -1;
    
    // Check if touch is in list area (0-205px wide, 40-235px tall)
    if (g_touchPos.px < 5 || g_touchPos.px > 210 || g_touchPos.py < 40 || g_touchPos.py > 235) {
        return -1;
    }
    
    float list_y = 40.0f;
    float list_item_h = 16.0f;
    
    int item_idx = (int)((g_touchPos.py - list_y) / list_item_h);
    
    if (item_idx >= 0 && item_idx < g_num_available_shows && item_idx < 12) {
        return item_idx;
    }
    return -1;
}
