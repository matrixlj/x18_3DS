#include "common.h"
#include "show_manager_window.h"
#include "network_config_window.h"
#include "options_window.h"

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
    C2D_SceneBegin(g_botScreen.target);
    C2D_DrawRectSolid(0.0f, 0.0f, 0.5f, SCREEN_WIDTH_BOT, SCREEN_HEIGHT_BOT, CLR_BG_PRIMARY);
    
    // Header panel
    draw_panel_header(0.0f, 0.0f, SCREEN_WIDTH_BOT, 35.0f, "Show Manager", CLR_BORDER_CYAN);
    draw_debug_text(&g_botScreen, "Show Manager", 5, 10, 0.45f, CLR_TEXT_PRIMARY);
    
    // List of shows (starts at y=40)
    float list_y = 40.0f;
    float list_item_h = 20.0f;
    u32 clrModified = CLR_BORDER_YELLOW;
    
    for (int i = 0; i < g_num_available_shows && i < 9; i++) {
        draw_list_item_bg(0.0f, list_y, SCREEN_WIDTH_BOT, list_item_h, (i == g_selected_show));
        
        u32 text_color = (i == g_selected_show) ? CLR_TEXT_PRIMARY : CLR_TEXT_SECONDARY;
        
        // Check if modified
        if (g_show_modified && strcmp(g_available_shows[i], g_current_show.name) == 0) {
            text_color = clrModified;
        }
        
        // Renaming mode
        if (g_renaming && i == g_selected_show) {
            draw_debug_text(&g_botScreen, g_new_name, 15, list_y + 2, 0.35f, CLR_BORDER_CYAN);
            draw_debug_text(&g_botScreen, "_", 15 + g_rename_input_pos * 8, list_y + 2, 0.35f, CLR_BORDER_CYAN);
        } else {
            draw_debug_text(&g_botScreen, g_available_shows[i], 15, list_y + 2, 0.35f, text_color);
        }
        
        list_y += list_item_h;
    }
    
    // Bottom buttons
    float btn_row1_y = 205.0f;
    float btn_row2_y = 221.0f;
    float btn_width_4 = SCREEN_WIDTH_BOT / 4.0f;
    float btn_width_3 = SCREEN_WIDTH_BOT / 3.0f;
    float btn_h = 14.0f;
    
    const char *btn_labels_row1[] = {"LOAD", "DEL", "DUP", "REN"};
    const char *btn_labels_row2[] = {"OPT", "NET", "EXIT"};
    
    // Row 1: 4 buttons (LOAD, DEL, DUP, REN)
    for (int i = 0; i < 4; i++) {
        float btn_x = i * btn_width_4;
        draw_button_3d(btn_x, btn_row1_y, btn_width_4 - 1, btn_h, CLR_BG_SECONDARY);
        draw_debug_text(&g_botScreen, btn_labels_row1[i], btn_x + 12, btn_row1_y - 2, 0.35f, CLR_TEXT_PRIMARY);
    }
    
    // Row 2: 3 buttons (OPT, NET, EXIT)
    u32 colors_row2[] = {CLR_BORDER_ORANGE, CLR_BORDER_CYAN, CLR_BORDER_YELLOW};
    for (int i = 0; i < 3; i++) {
        float btn_x = i * btn_width_3;
        draw_button_3d(btn_x, btn_row2_y, btn_width_3 - 1, btn_h, colors_row2[i]);
        draw_debug_text(&g_botScreen, btn_labels_row2[i], btn_x + 16, btn_row2_y - 2, 0.35f, CLR_TEXT_PRIMARY);
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
                    snprintf(new_name, sizeof(new_name), "%s_copy", g_available_shows[g_selected_show]);
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
    
    // Button layout (2 rows):
    // Row 1 (y=210): LOAD(0), DEL(1), DUP(2), REN(3) - 4 buttons
    // Row 2 (y=225): OPT(4), NET(5), EXIT(6) - 3 buttons
    
    float btn_row1_y = 210.0f;
    float btn_row2_y = 225.0f;
    float btn_width_4 = SCREEN_WIDTH_BOT / 4.0f;
    float btn_width_3 = SCREEN_WIDTH_BOT / 3.0f;
    
    if (button_idx >= 0 && button_idx < 4) {
        // Row 1 buttons
        float btn_x = button_idx * btn_width_4;
        return (g_touchPos.px >= btn_x && g_touchPos.px < btn_x + btn_width_4 &&
                g_touchPos.py >= btn_row1_y && g_touchPos.py < btn_row1_y + 14);
    } else if (button_idx >= 4 && button_idx < 7) {
        // Row 2 buttons (map indices 4,5,6 to positions 0,1,2)
        int row2_idx = button_idx - 4;
        float btn_x = row2_idx * btn_width_3;
        return (g_touchPos.px >= btn_x && g_touchPos.px < btn_x + btn_width_3 &&
                g_touchPos.py >= btn_row2_y && g_touchPos.py < btn_row2_y + 14);
    }
    
    return 0;
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
