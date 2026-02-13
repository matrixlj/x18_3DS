#include "common.h"
#include "show_manager_window.h"
#include "network_config_window.h"

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
    u32 clrBg = C2D_Color32(0x20, 0x20, 0x20, 0xFF);
    C2D_TargetClear(g_botScreen.target, clrBg);
    C2D_SceneBegin(g_botScreen.target);
    
    u32 clrText = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    u32 clrSelected = C2D_Color32(0xFF, 0xFF, 0x00, 0xFF);
    u32 clrBorder = C2D_Color32(0x50, 0x50, 0x50, 0xFF);
    u32 clrBtnBg = C2D_Color32(0x40, 0x40, 0x40, 0xFF);
    
    // Title
    draw_debug_text(&g_botScreen, "Show Manager", 5, 5, 0.4f, clrText);
    
    // List of shows (starts at y=25)
    float list_y = 25.0f;
    u32 clrNotSaved = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);  // Red for unsaved
    
    for (int i = 0; i < g_num_available_shows && i < 10; i++) {
        u32 color = (i == g_selected_show) ? clrSelected : clrText;
        
        // Check if this show is the current one and not saved
        if (g_show_modified && strcmp(g_available_shows[i], g_current_show.name) == 0) {
            color = clrNotSaved;
        }
        
        // Show item background
        if (i == g_selected_show) {
            C2D_DrawRectSolid(0, list_y, 0.5f, SCREEN_WIDTH_BOT, 18, C2D_Color32(0x30, 0x30, 0x60, 0xFF));
        }
        
        // Check if renaming this item
        if (g_renaming && i == g_selected_show) {
            draw_debug_text(&g_botScreen, g_new_name, 10, list_y + 2, 0.35f, clrSelected);
            draw_debug_text(&g_botScreen, "_", 10 + g_rename_input_pos * 8, list_y + 2, 0.35f, clrSelected);
        } else {
            draw_debug_text(&g_botScreen, g_available_shows[i], 10, list_y + 2, 0.35f, color);
        }
        
        list_y += 18.0f;
    }
    
    // Bottom buttons (6 buttons on 2 rows: 4 on first row, 2 on second)
    float btn_row1_y = 210.0f;
    float btn_row2_y = 225.0f;
    float btn_width_4 = SCREEN_WIDTH_BOT / 4.0f;   // Width for 4 buttons
    float btn_width_2 = SCREEN_WIDTH_BOT / 2.0f;   // Width for 2 buttons
    
    const char *btn_labels_row1[] = {"LOAD", "DEL", "DUP", "REN"};
    const char *btn_labels_row2[] = {"NET", "EXIT"};
    u32 clrExit = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);  // Red for exit button
    u32 clrNet = C2D_Color32(0x00, 0xFF, 0xFF, 0xFF);   // Cyan for net button
    
    // Row 1: 4 buttons (LOAD, DEL, DUP, REN)
    for (int i = 0; i < 4; i++) {
        float btn_x = i * btn_width_4;
        C2D_DrawRectSolid(btn_x, btn_row1_y, 0.5f, btn_width_4 - 1, 14, clrBtnBg);
        C2D_DrawRectangle(btn_x, btn_row1_y, 0.5f, btn_width_4 - 1, 14, clrBorder, clrBorder, clrBorder, clrBorder);
        draw_debug_text(&g_botScreen, btn_labels_row1[i], btn_x + 4, btn_row1_y - 1, 0.4f, clrText);
    }
    
    // Row 2: 2 buttons (NET, EXIT)
    for (int i = 0; i < 2; i++) {
        float btn_x = i * btn_width_2;
        C2D_DrawRectSolid(btn_x, btn_row2_y, 0.5f, btn_width_2 - 1, 14, clrBtnBg);
        C2D_DrawRectangle(btn_x, btn_row2_y, 0.5f, btn_width_2 - 1, 14, clrBorder, clrBorder, clrBorder, clrBorder);
        
        u32 btn_color = clrText;
        if (i == 0) btn_color = clrNet;   // Cyan for NET
        else if (i == 1) btn_color = clrExit;  // Red for EXIT
        
        draw_debug_text(&g_botScreen, btn_labels_row2[i], btn_x + 10, btn_row2_y - 1, 0.4f, btn_color);
    }
}

void handle_manager_input(void)
{
    u32 kDown = hidKeysDown();
    int touch_edge = g_isTouched && !g_wasTouched;
    
    // If network config window is open, handle its input and disable touch
    if (g_net_config_open) {
        handle_net_config_input(kDown);
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
            } else if (check_button_touch(BTN_NET)) {
                // NET button - open network configuration window
                g_net_config_open = 1;
                g_net_selected_field = 0;
                g_net_digit_index = 0;
            } else if (check_button_touch(5)) {  // Button 5 = EXIT
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
    // Row 2 (y=225): NET(4), EXIT(5) - 2 buttons
    
    float btn_row1_y = 210.0f;
    float btn_row2_y = 225.0f;
    float btn_width_4 = SCREEN_WIDTH_BOT / 4.0f;
    float btn_width_2 = SCREEN_WIDTH_BOT / 2.0f;
    
    if (button_idx >= 0 && button_idx < 4) {
        // Row 1 buttons
        float btn_x = button_idx * btn_width_4;
        return (g_touchPos.px >= btn_x && g_touchPos.px < btn_x + btn_width_4 &&
                g_touchPos.py >= btn_row1_y && g_touchPos.py < btn_row1_y + 14);
    } else if (button_idx >= 4 && button_idx < 6) {
        // Row 2 buttons (map indices 4,5 to positions 0,1)
        int row2_idx = button_idx - 4;
        float btn_x = row2_idx * btn_width_2;
        return (g_touchPos.px >= btn_x && g_touchPos.px < btn_x + btn_width_2 &&
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
