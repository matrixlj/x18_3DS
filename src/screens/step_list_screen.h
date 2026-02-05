#ifndef STEP_LIST_SCREEN_H
#define STEP_LIST_SCREEN_H

#include "../core/state.h"

// Step list display configuration
#define STEP_LIST_ITEM_HEIGHT 30
#define STEP_LIST_TOP_MARGIN 20
#define STEP_LIST_LEFT_MARGIN 10
#define STEP_LIST_RIGHT_MARGIN 10
#define STEP_LIST_VISIBLE_ITEMS 6  // Can fit ~6 items on 400x240 screen

// Step list rendering functions
void step_list_render_screen(AppState_Global *state);
void step_list_render_item(int x, int y, int step_num, const char *step_name, int selected);
void step_list_render_info_bar(AppState_Global *state);

// Step list input handling
void step_list_handle_button_input(AppState_Global *state, int button);

// Navigation utilities
int step_list_get_display_offset(int selected_step, int visible_items);

#endif // STEP_LIST_SCREEN_H
