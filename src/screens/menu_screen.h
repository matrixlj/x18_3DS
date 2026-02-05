#ifndef MENU_SCREEN_H
#define MENU_SCREEN_H

#include "../core/state.h"

// Menu types
typedef enum {
    MENU_MAIN,
    MENU_LOAD_SHOW,
    MENU_CREATE_SHOW,
    MENU_IP_CONFIG
} MenuType;

// Menu rendering functions
void menu_render_main(AppState_Global *state);
void menu_render_load_show(AppState_Global *state);
void menu_render_create_show(AppState_Global *state);
void menu_render_ip_config(AppState_Global *state);

// Menu input handling
void menu_handle_input(AppState_Global *state, int button);

// Menu utilities
void menu_draw_box(int x1, int y1, int x2, int y2, unsigned short color);
void menu_draw_text_simple(int x, int y, const char *text, unsigned short color);
void menu_draw_button(int x, int y, int w, int h, const char *label, int selected, unsigned short color);

#endif // MENU_SCREEN_H
