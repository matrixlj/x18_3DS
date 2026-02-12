#include "common.h"
#include "keyboard_window.h"

void render_keyboard(void)
{
    u32 clrBg = C2D_Color32(0x20, 0x20, 0x20, 0xFF);
    u32 clrBorder = C2D_Color32(0x50, 0x50, 0x50, 0xFF);
    u32 clrWhite = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    u32 clrRed = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
    u32 clrGreen = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);
    u32 clrYellow = C2D_Color32(0xFF, 0xFF, 0x00, 0xFF);
    u32 clrCyan = C2D_Color32(0x00, 0xFF, 0xFF, 0xFF);
    
    C2D_TargetClear(g_botScreen.target, clrBg);
    C2D_SceneBegin(g_botScreen.target);
    
    // Title
    draw_debug_text(&g_botScreen, "Virtual Keyboard", 10, 5, 0.4f, clrCyan);
    
    // Input display
    C2D_DrawRectangle(10, 20, 0.5f, SCREEN_WIDTH_BOT - 20, 30, clrBorder, clrBorder, clrBorder, clrBorder);
    draw_debug_text(&g_botScreen, g_new_show_name, 15, 25, 0.4f, clrYellow);
    
    // Virtual keyboard rows
    float kb_y = 55.0f;
    float key_width = (SCREEN_WIDTH_BOT - 20) / 10.0f;
    
    // Row 1: QWERTYUIOP
    const char *row1 = "QWERTYUIOP";
    for (int i = 0; i < 10; i++) {
        char key_char[2] = {row1[i], '\0'};
        float key_x = 10.0f + i * key_width;
        C2D_DrawRectangle(key_x, kb_y, 0.5f, key_width - 2, 20, clrBorder, clrBorder, clrBorder, clrBorder);
        draw_debug_text(&g_botScreen, key_char, key_x + 4, kb_y + 2, 0.35f, clrWhite);
    }
    
    // Row 2: ASDFGHJKL
    const char *row2 = "ASDFGHJKL";
    kb_y += 22.0f;
    for (int i = 0; i < 9; i++) {
        char key_char[2] = {row2[i], '\0'};
        float key_x = 10.0f + (i + 0.5f) * key_width;
        C2D_DrawRectangle(key_x, kb_y, 0.5f, key_width - 2, 20, clrBorder, clrBorder, clrBorder, clrBorder);
        draw_debug_text(&g_botScreen, key_char, key_x + 4, kb_y + 2, 0.35f, clrWhite);
    }
    
    // Row 3: ZXCVBNM
    const char *row3 = "ZXCVBNM";
    kb_y += 22.0f;
    for (int i = 0; i < 7; i++) {
        char key_char[2] = {row3[i], '\0'};
        float key_x = 10.0f + (i + 1.5f) * key_width;
        C2D_DrawRectangle(key_x, kb_y, 0.5f, key_width - 2, 20, clrBorder, clrBorder, clrBorder, clrBorder);
        draw_debug_text(&g_botScreen, key_char, key_x + 4, kb_y + 2, 0.35f, clrWhite);
    }
    
    // Row 4: Action buttons
    kb_y += 22.0f;
    float btn_width = (SCREEN_WIDTH_BOT - 20) / 4.0f;
    
    // Backspace
    C2D_DrawRectangle(10, kb_y, 0.5f, btn_width - 2, 20, clrBorder, clrBorder, clrBorder, clrBorder);
    draw_debug_text(&g_botScreen, "<--", 15, kb_y + 2, 0.3f, clrRed);
    
    // Space
    C2D_DrawRectangle(10 + btn_width, kb_y, 0.5f, btn_width - 2, 20, clrBorder, clrBorder, clrBorder, clrBorder);
    draw_debug_text(&g_botScreen, "SPC", 10 + btn_width + 5, kb_y + 2, 0.3f, clrWhite);
    
    // OK
    C2D_DrawRectangle(10 + btn_width * 2, kb_y, 0.5f, btn_width - 2, 20, clrBorder, clrBorder, clrBorder, clrBorder);
    draw_debug_text(&g_botScreen, "OK", 10 + btn_width * 2 + 12, kb_y + 2, 0.35f, clrGreen);
    
    // Cancel
    C2D_DrawRectangle(10 + btn_width * 3, kb_y, 0.5f, btn_width - 2, 20, clrBorder, clrBorder, clrBorder, clrBorder);
    draw_debug_text(&g_botScreen, "Esc", 10 + btn_width * 3 + 10, kb_y + 2, 0.3f, clrRed);
}

void handle_keyboard_input(char c)
{
    size_t len = strlen(g_new_show_name);
    if (len < 63) {
        g_new_show_name[len] = c;
        g_new_show_name[len + 1] = '\0';
    }
}

// Forward declarations from external
extern void init_new_show(const char *name);
extern void save_show_to_file(Show *show);
extern void list_available_shows(void);

void handle_new_show_input(void)
{
    u32 kDown = hidKeysDown();
    int touch_edge = g_isTouched && !g_wasTouched;
    
    // Handle button press input (A/B for quick confirm/cancel)
    if (kDown & KEY_A) {
        // Confirm
        if (strlen(g_new_show_name) > 0) {
            g_new_show_name[63] = '\0';  // Ensure null termination
            init_new_show(g_new_show_name);  // Create new show with 3 default steps
            save_show_to_file(&g_current_show);  // Save immediately
            
            snprintf(g_save_status, sizeof(g_save_status), "OK: '%s' created", g_new_show_name);
            g_save_status_timer = 120;
            
            g_creating_new_show = 0;
            memset(g_new_show_name, 0, sizeof(g_new_show_name));
            list_available_shows();
        }
        return;
    } else if (kDown & KEY_B) {
        // Cancel
        g_creating_new_show = 0;
        memset(g_new_show_name, 0, sizeof(g_new_show_name));
        return;
    }
    
    // Handle touch input on virtual keyboard
    if (touch_edge) {
        float touch_x = (float)g_touchPos.px;
        float touch_y = (float)g_touchPos.py;
        
        // Keyboard layout on bottom screen
        float key_width = (SCREEN_WIDTH_BOT - 20.0f) / 10.0f;
        
        // Row 1: QWERTYUIOP (y=55 to 75)
        if (touch_y >= 55.0f && touch_y < 75.0f) {
            if (touch_x >= 10.0f && touch_x < SCREEN_WIDTH_BOT - 10.0f) {
                int key_idx = (int)((touch_x - 10.0f) / key_width);
                if (key_idx >= 0 && key_idx < 10) {
                    const char *row1 = "QWERTYUIOP";
                    char key_char[2] = {row1[key_idx], '\0'};
                    handle_keyboard_input(key_char[0]);
                }
            }
        }
        // Row 2: ASDFGHJKL (y=77 to 97)
        else if (touch_y >= 77.0f && touch_y < 97.0f) {
            if (touch_x >= 10.0f && touch_x < SCREEN_WIDTH_BOT - 10.0f) {
                int key_idx = (int)((touch_x - 10.0f) / key_width);
                if (key_idx >= 0 && key_idx < 9) {
                    const char *row2 = "ASDFGHJKL";
                    char key_char[2] = {row2[key_idx], '\0'};
                    handle_keyboard_input(key_char[0]);
                }
            }
        }
        // Row 3: ZXCVBNM (y=99 to 119)
        else if (touch_y >= 99.0f && touch_y < 119.0f) {
            if (touch_x >= 10.0f && touch_x < SCREEN_WIDTH_BOT - 10.0f) {
                int key_idx = (int)((touch_x - 10.0f) / key_width);
                if (key_idx >= 1 && key_idx < 8) {  // Offset by 1.5
                    const char *row3 = "ZXCVBNM";
                    char key_char[2] = {row3[key_idx - 1], '\0'};
                    handle_keyboard_input(key_char[0]);
                }
            }
        }
        // Row 4: Action buttons (y=121 to 141)
        else if (touch_y >= 121.0f && touch_y < 141.0f) {
            float btn_width = (SCREEN_WIDTH_BOT - 20.0f) / 4.0f;
            
            // Backspace (x=10 to btn_width+10)
            if (touch_x >= 10.0f && touch_x < 10.0f + btn_width) {
                // Delete last character
                if (strlen(g_new_show_name) > 0) {
                    g_new_show_name[strlen(g_new_show_name) - 1] = '\0';
                }
            }
            // Space (x=btn_width+10 to 2*btn_width+10)
            else if (touch_x >= 10.0f + btn_width && touch_x < 10.0f + btn_width * 2) {
                handle_keyboard_input(' ');
            }
            // OK (x=2*btn_width+10 to 3*btn_width+10)
            else if (touch_x >= 10.0f + btn_width * 2 && touch_x < 10.0f + btn_width * 3) {
                // Confirm
                if (strlen(g_new_show_name) > 0) {
                    g_new_show_name[63] = '\0';
                    init_new_show(g_new_show_name);  // Create new show with 3 default steps
                    save_show_to_file(&g_current_show);  // Save immediately
                    
                    snprintf(g_save_status, sizeof(g_save_status), "OK: '%s' created", g_new_show_name);
                    g_save_status_timer = 120;
                    
                    g_creating_new_show = 0;
                    memset(g_new_show_name, 0, sizeof(g_new_show_name));
                    list_available_shows();
                }
            }
            // Cancel (x=3*btn_width+10 to 4*btn_width+10)
            else if (touch_x >= 10.0f + btn_width * 3 && touch_x < 10.0f + btn_width * 4) {
                g_creating_new_show = 0;
                memset(g_new_show_name, 0, sizeof(g_new_show_name));
            }
        }
    }
}
