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
    
    // Full screen background
    u32 clrBg = C2D_Color32(0x1A, 0x2A, 0x3A, 0xFF);
    u32 clrBorder = C2D_Color32(0x80, 0x80, 0xFF, 0xFF);
    u32 clrLabel = C2D_Color32(0x80, 0xFF, 0xFF, 0xFF);
    u32 clrText = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    u32 clrTitle = C2D_Color32(0xFF, 0xFF, 0x00, 0xFF);
    
    C2D_DrawRectSolid(0, 0, 0.40f, SCREEN_WIDTH_TOP, SCREEN_HEIGHT_TOP, clrBg);
    C2D_DrawRectangle(0, 0, 0.40f, SCREEN_WIDTH_TOP, SCREEN_HEIGHT_TOP, clrBorder, clrBorder, clrBorder, clrBorder);
    
    // ===== HEADER BAR WITH SHOW NAME =====
    C2D_DrawRectSolid(0, 0, 0.45f, SCREEN_WIDTH_TOP, 50, C2D_Color32(0x0F, 0x0F, 0x3F, 0xFF));
    C2D_DrawRectangle(0, 0, 0.45f, SCREEN_WIDTH_TOP, 50, clrBorder, clrBorder, clrBorder, clrBorder);
    
    draw_debug_text(&g_topScreen, "Show Info", 15.0f, 8.0f, 0.45f, clrLabel);
    draw_debug_text(&g_topScreen, g_available_shows[g_selected_show], 15.0f, 25.0f, 0.5f, clrTitle);
    
    // ===== SHOW DETAILS =====
    C2D_DrawRectSolid(10, 60, 0.45f, SCREEN_WIDTH_TOP - 20, SCREEN_HEIGHT_TOP - 80, C2D_Color32(0x15, 0x15, 0x25, 0xFF));
    C2D_DrawRectangle(10, 60, 0.45f, SCREEN_WIDTH_TOP - 20, SCREEN_HEIGHT_TOP - 80, clrBorder, clrBorder, clrBorder, clrBorder);
    
    float info_y = 70.0f;
    
    // Number of steps
    draw_debug_text(&g_topScreen, "Steps:", 20.0f, info_y, 0.4f, clrLabel);
    char step_count[32];
    snprintf(step_count, sizeof(step_count), "%d", g_current_show.num_steps);
    draw_debug_text(&g_topScreen, step_count, 100.0f, info_y, 0.4f, clrText);
    info_y += 25.0f;
    
    // File path
    draw_debug_text(&g_topScreen, "Path:", 20.0f, info_y, 0.4f, clrLabel);
    char path_display[48];
    snprintf(path_display, sizeof(path_display), "/3ds/x18mixer/shows/");
    draw_debug_text(&g_topScreen, path_display, 20.0f, info_y + 18.0f, 0.4f, clrText);
    info_y += 40.0f;
    
    char filename[128];
    snprintf(filename, sizeof(filename), "%s.x18s", g_available_shows[g_selected_show]);
    draw_debug_text(&g_topScreen, filename, 20.0f, info_y, 0.4f, clrText);
    info_y += 25.0f;
    
    // Channels (16)
    draw_debug_text(&g_topScreen, "Channels:", 20.0f, info_y, 0.4f, clrLabel);
    draw_debug_text(&g_topScreen, "16 (Mono Channels)", 100.0f, info_y, 0.4f, clrText);
    info_y += 25.0f;
    
    // EQ Bands (5 per channel)
    draw_debug_text(&g_topScreen, "EQ Bands/Ch:", 20.0f, info_y, 0.4f, clrLabel);
    draw_debug_text(&g_topScreen, "5 (Parametric)", 100.0f, info_y, 0.4f, clrText);
    info_y += 25.0f;
    
    // Status message
    if (g_save_status_timer > 0) {
        C2D_DrawRectSolid(10, SCREEN_HEIGHT_TOP - 35, 0.45f, SCREEN_WIDTH_TOP - 20, 25, C2D_Color32(0x00, 0x33, 0x00, 0xFF));
        draw_debug_text(&g_topScreen, g_save_status, 20.0f, SCREEN_HEIGHT_TOP - 30, 0.35f, CLR_GREEN);
    } else {
        // Instructions
        draw_debug_text(&g_topScreen, "A:Load  B:Delete  Y:Duplicate  X:Rename", 15.0f, SCREEN_HEIGHT_TOP - 30, 0.4f, clrLabel);
    }
}
