#ifndef EQ_WINDOW_H
#define EQ_WINDOW_H

#include "common.h"

// ============================================================================
// EQ WINDOW FUNCTIONS
// ============================================================================

void render_eq_window(void);
void render_eq_info_panel(void);
void handle_eq_input(u32 kDown, u32 kHeld);
void update_eq_touch(void);

// ============================================================================
// EQ HELPER FUNCTIONS
// ============================================================================

const char* get_filter_type_name(EQFilterType type);
float calculate_eq_response(EQBand *band, float freq_hz);

#endif
