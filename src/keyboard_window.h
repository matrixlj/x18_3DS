#ifndef KEYBOARD_WINDOW_H
#define KEYBOARD_WINDOW_H

#include "common.h"

// ============================================================================
// KEYBOARD WINDOW STATE
// ============================================================================

extern int g_show_keyboard;

// ============================================================================
// KEYBOARD WINDOW FUNCTIONS
// ============================================================================

void render_keyboard(void);
void handle_keyboard_input(char c);
void handle_new_show_input(void);

#endif
