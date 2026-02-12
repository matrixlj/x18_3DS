#ifndef MIXER_WINDOW_H
#define MIXER_WINDOW_H

#include "common.h"

// ============================================================================
// EQ WINDOW STATE
// ============================================================================

extern int g_eq_window_open;
extern int g_eq_editing_channel;
extern int g_eq_selected_band;
extern int g_eq_param_selected;

// ============================================================================
// EQ WINDOW FUNCTIONS
// ============================================================================

void render_eq_window(void);
void render_eq_info_panel(void);
void handle_eq_input(u32 kDown, u32 kHeld);
float fader_value_to_db(float value);
float calculate_eq_response(EQBand *band, float freq_hz);
const char* get_filter_type_name(EQFilterType type);

#endif
