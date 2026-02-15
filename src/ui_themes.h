/*
 * ui_themes.h - Modern 3D UI theme system for X18 Mixer
 * Dark palette with high contrast for low-light usage
 */

#ifndef UI_THEMES_H
#define UI_THEMES_H

#include <citro2d.h>

// ============= COLOR PALETTE =============
// Dark theme with high contrast

#define CLR_BG_PRIMARY      C2D_Color32(0x0D, 0x0D, 0x12, 0xFF)  // Very dark blue-black
#define CLR_BG_SECONDARY    C2D_Color32(0x20, 0x20, 0x2A, 0xFF)  // Medium dark
#define CLR_BG_TERTIARY     C2D_Color32(0x32, 0x32, 0x41, 0xFF)  // Light dark

#define CLR_BORDER_BRIGHT   C2D_Color32(0xC8, 0xC8, 0xFF, 0xFF)  // Bright blue
#define CLR_BORDER_CYAN     C2D_Color32(0x00, 0xFF, 0xFF, 0xFF)  // Cyan
#define CLR_BORDER_YELLOW   C2D_Color32(0xFF, 0xFF, 0x00, 0xFF)  // Yellow
#define CLR_BORDER_ORANGE   C2D_Color32(0xFF, 0x99, 0x00, 0xFF)  // Orange
#define CLR_BORDER_GREEN    C2D_Color32(0x00, 0xFF, 0x00, 0xFF)  // Green

#define CLR_TEXT_PRIMARY    C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF)  // White
#define CLR_TEXT_SECONDARY  C2D_Color32(0xC8, 0xC8, 0xC8, 0xFF)  // Light gray

#define CLR_SHADOW_BLACK    C2D_Color32(0x00, 0x00, 0x00, 0xFF)  // Black
#define CLR_HIGHLIGHT_BLUE  C2D_Color32(0x64, 0xC8, 0xFF, 0xFF)  // Light blue

// ============= UI UTILITIES =============

/**
 * Draw a 3D beveled border for modern look
 * Creates inset/outset effect with bright top-left and dark bottom-right
 */
static inline void draw_3d_border(
    float x, float y, float w, float h,
    u32 color_bright, u32 color_dark,
    int thickness)
{
    // Top and left sides (bright)
    C2D_DrawRectSolid(x, y, 0.5f, w, thickness, color_bright);
    C2D_DrawRectSolid(x, y, 0.5f, thickness, h, color_bright);
    
    // Bottom and right sides (dark)
    C2D_DrawRectSolid(x, y + h - thickness, 0.5f, w, thickness, color_dark);
    C2D_DrawRectSolid(x + w - thickness, y, 0.5f, thickness, h, color_dark);
}

/**
 * Draw a 3D button in modern style
 */
static inline void draw_button_3d(
    float x, float y, float w, float h,
    u32 color_bg)
{
    // Main button surface
    C2D_DrawRectSolid(x, y, 0.5f, w, h, color_bg);
    
    // 3D borders
    draw_3d_border(x, y, w, h, CLR_BORDER_BRIGHT, CLR_SHADOW_BLACK, 2);
    
    // Inner highlight for depth
    C2D_DrawRectSolid(x + 2, y + 2, 0.5f, w - 4, 1, CLR_HIGHLIGHT_BLUE);
}

/**
 * Draw a modern panel with title header
 */
static inline void draw_panel_header(
    float x, float y, float w, float h,
    const char *title,
    u32 border_color)
{
    // Header background gradient effect (simulated with rectangles)
    for (int i = 0; i < h; i++) {
        float ratio = (float)i / h;
        u32 r = (u32)(CLR_BG_SECONDARY >> 0 & 0xFF) + (u32)((CLR_BG_TERTIARY >> 0 & 0xFF) - (CLR_BG_SECONDARY >> 0 & 0xFF)) * ratio;
        u32 g = (u32)(CLR_BG_SECONDARY >> 8 & 0xFF) + (u32)((CLR_BG_TERTIARY >> 8 & 0xFF) - (CLR_BG_SECONDARY >> 8 & 0xFF)) * ratio;
        u32 b = (u32)(CLR_BG_SECONDARY >> 16 & 0xFF) + (u32)((CLR_BG_TERTIARY >> 16 & 0xFF) - (CLR_BG_SECONDARY >> 16 & 0xFF)) * ratio;
        u32 color_line = C2D_Color32(r, g, b, 0xFF);
        C2D_DrawRectSolid(x, y + i, 0.5f, w, 1, color_line);
    }
    
    // Bottom bright line for depth effect
    C2D_DrawRectSolid(x, y + h - 2, 0.5f, w, 2, border_color);
    
    // Title text (caller should handle after this)
}

/**
 * Draw list item with highlight if selected
 */
static inline void draw_list_item_bg(
    float x, float y, float w, float h,
    int selected)
{
    u32 bg_color = selected ? CLR_BORDER_CYAN : CLR_BG_SECONDARY;
    
    // Item background
    C2D_DrawRectSolid(x, y, 0.5f, w, h, bg_color);
    
    if (selected) {
        // Bright border for selection
        draw_3d_border(x, y, w, h, CLR_BORDER_YELLOW, CLR_BG_PRIMARY, 1);
    } else {
        // Subtle border
        C2D_DrawRectangle(x, y, 0.5f, w, h, CLR_BG_TERTIARY, CLR_BG_TERTIARY, CLR_BG_TERTIARY, CLR_BG_TERTIARY);
    }
    
    // Left accent bar
    C2D_DrawRectSolid(x, y, 0.5f, 3, h, bg_color);
}

/**
 * Draw modern input field (recessed effect)
 */
static inline void draw_input_field(
    float x, float y, float w, float h)
{
    // Field background
    C2D_DrawRectSolid(x, y, 0.5f, w, h, CLR_BG_PRIMARY);
    
    // Recessed 3D border (inset)
    draw_3d_border(x, y, w, h, CLR_SHADOW_BLACK, CLR_BORDER_CYAN, 2);
    
    // Inner border
    C2D_DrawRectangle(x + 2, y + 2, 0.5f, w - 4, h - 4, CLR_BORDER_BRIGHT, CLR_BORDER_BRIGHT, CLR_BORDER_BRIGHT, CLR_BORDER_BRIGHT);
}

/**
 * Draw keyboard key in 3D style
 */
static inline void draw_key_3d(
    float x, float y, float w, float h,
    u32 key_color)
{
    // Key button
    C2D_DrawRectSolid(x + 1, y + 1, 0.5f, w - 2, h - 2, key_color);
    
    // 3D border
    draw_3d_border(x + 1, y + 1, w - 2, h - 2, CLR_BORDER_BRIGHT, CLR_SHADOW_BLACK, 1);
}

/**
 * Draw progress/indicator bar
 */
static inline void draw_progress_bar(
    float x, float y, float w, float h,
    float fill_ratio,
    u32 color_fill)
{
    // Container background
    C2D_DrawRectSolid(x, y, 0.5f, w, h, CLR_BG_SECONDARY);
    
    // 3D border for container
    draw_3d_border(x, y, w, h, CLR_BORDER_BRIGHT, CLR_SHADOW_BLACK, 2);
    
    // Fill bar
    float fill_w = (w - 4) * fill_ratio;
    if (fill_w > 0) {
        C2D_DrawRectSolid(x + 2, y + 2, 0.5f, fill_w, h - 4, color_fill);
    }
}

#endif // UI_THEMES_H
