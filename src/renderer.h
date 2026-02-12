#ifndef RENDERER_H
#define RENDERER_H

#include "common.h"

// ============================================================================
// RENDERING FUNCTIONS
// ============================================================================

void render_top_screen(void);
void render_bot_screen(void);
void render_frame(void);

// ============================================================================
// UTILITY FUNCTIONS (from main.c)
// ============================================================================

extern void draw_3d_button(float x, float y, float w, float h, u32 color_main, u32 color_light, u32 color_dark, int pressed);

#endif
