#include "common.h"
#include "show_info_panel.h"

// Color constants
#define CLR_GREEN C2D_Color32(0x00, 0xFF, 0x00, 0xFF)

void render_show_info_panel(void)
{
    if (g_app_mode != APP_MODE_MANAGER || g_selected_show < 0 || g_selected_show >= g_num_available_shows) {
        return;
    }
    
    C2D_SceneBegin(g_topScreen.target);
    C2D_DrawRectSolid(0, 0, 0.40f, SCREEN_WIDTH_TOP, SCREEN_HEIGHT_TOP, CLR_BG_PRIMARY);
    
    // ===== HEADER: Show Name =====
    draw_panel_header(10, 10, SCREEN_WIDTH_TOP - 20, 45, "Show Details", CLR_BORDER_CYAN);
    draw_debug_text(&g_topScreen, g_available_shows[g_selected_show], 25.0f, 20.0f, 0.55f, CLR_BORDER_YELLOW);
    
    // ===== INFO GRID (Clean layout) =====
    float col1_x = 25.0f;
    float col2_x = 140.0f;
    float info_y = 70.0f;
    float info_spacing = 30.0f;
    
    // Row 1: Steps
    draw_debug_text(&g_topScreen, "Steps:", col1_x, info_y, 0.45f, CLR_TEXT_PRIMARY);
    char step_count[16];
    snprintf(step_count, sizeof(step_count), "%d", g_current_show.num_steps);
    draw_debug_text(&g_topScreen, step_count, col2_x, info_y, 0.45f, CLR_BORDER_CYAN);
    
    // Row 2: Channels
    info_y += info_spacing;
    draw_debug_text(&g_topScreen, "Channels:", col1_x, info_y, 0.45f, CLR_TEXT_PRIMARY);
    draw_debug_text(&g_topScreen, "16", col2_x, info_y, 0.45f, CLR_BORDER_ORANGE);
    
    // Row 3: EQ Bands
    info_y += info_spacing;
    draw_debug_text(&g_topScreen, "EQ Bands:", col1_x, info_y, 0.45f, CLR_TEXT_PRIMARY);
    draw_debug_text(&g_topScreen, "5/Ch", col2_x, info_y, 0.45f, CLR_BORDER_GREEN);
    
    // ===== PATH PANEL (Small info at bottom) =====
    C2D_DrawRectSolid(15, SCREEN_HEIGHT_TOP - 50, 0.45f, SCREEN_WIDTH_TOP - 30, 45, CLR_BG_SECONDARY);
    draw_3d_border(15, SCREEN_HEIGHT_TOP - 50, SCREEN_WIDTH_TOP - 30, 45, CLR_BORDER_BRIGHT, CLR_SHADOW_BLACK, 1);
    
    draw_debug_text(&g_topScreen, "File:", 25.0f, SCREEN_HEIGHT_TOP - 42, 0.35f, CLR_TEXT_SECONDARY);
    char filename[64];
    snprintf(filename, sizeof(filename), "%s.x18s", g_available_shows[g_selected_show]);
    draw_debug_text(&g_topScreen, filename, 25.0f, SCREEN_HEIGHT_TOP - 28, 0.35f, CLR_TEXT_PRIMARY);
    
    draw_debug_text(&g_topScreen, "Path: /3ds/x18mixer/shows/", 25.0f, SCREEN_HEIGHT_TOP - 12, 0.28f, CLR_TEXT_SECONDARY);
}
