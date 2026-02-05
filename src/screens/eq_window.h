#ifndef EQ_WINDOW_H
#define EQ_WINDOW_H

#include "../core/state.h"

// EQ Window Layout
#define EQ_WINDOW_WIDTH 280
#define EQ_WINDOW_HEIGHT 200
#define EQ_WINDOW_X 20
#define EQ_WINDOW_Y 20

#define EQ_BAND_ITEM_HEIGHT 30
#define EQ_BAND_DISPLAY_ITEMS 5

// EQ type strings (for display)
#define EQ_TYPE_LCUT 0
#define EQ_TYPE_LSHV 1
#define EQ_TYPE_PEQ 2
#define EQ_TYPE_VEQ 3
#define EQ_TYPE_HSHV 4
#define EQ_TYPE_HCUT 5

// EQ rendering functions
void eq_window_render_screen(AppState_Global *state);
void eq_window_render_header(AppState_Global *state);
void eq_window_render_bands(AppState_Global *state);
void eq_window_render_band_item(int y, int band_id, EQBandState *band, int selected);
void eq_window_render_param_editor(AppState_Global *state);

// EQ window input handling
void eq_window_handle_button_input(AppState_Global *state, int button);

// EQ parameter manipulation
void eq_window_adjust_parameter(AppState_Global *state, int delta);
void eq_window_cycle_band_type(AppState_Global *state);

// Utility
const char *eq_get_type_string(int type);
int eq_get_param_value(EQBandState *band, int param);
void eq_set_param_value(EQBandState *band, int param, int value);

#endif // EQ_WINDOW_H
