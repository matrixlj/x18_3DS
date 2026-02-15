#include "common.h"
#include "renderer.h"
#include "mixer_window.h"
#include "keyboard_window.h"
#include "show_manager_window.h"
#include "network_config_window.h"
#include "show_info_panel.h"
#include "eq_window.h"
#include "options_window.h"

// Color constants
#define CLR_BG_DARK C2D_Color32(0x1A, 0x1A, 0x1A, 0xFF)
#define CLR_BG_MID C2D_Color32(0x25, 0x25, 0x25, 0xFF)
#define CLR_BORDER C2D_Color32(0x40, 0x40, 0x40, 0xFF)
#define CLR_WHITE C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF)
#define CLR_YELLOW C2D_Color32(0xFF, 0xFF, 0x00, 0xFF)
#define CLR_CYAN C2D_Color32(0x00, 0xFF, 0xFF, 0xFF)
#define CLR_RED C2D_Color32(0xFF, 0x00, 0x00, 0xFF)
#define CLR_GREEN C2D_Color32(0x00, 0xFF, 0x00, 0xFF)

void render_top_screen(void)
{
    // Safety: Don't render if not fully initialized
    if (!g_init_complete) {
        C2D_TargetClear(g_topScreen.target, CLR_BG_DARK);
        C2D_SceneBegin(g_topScreen.target);
        return;
    }
    
    C2D_TargetClear(g_topScreen.target, CLR_BG_DARK);
    C2D_SceneBegin(g_topScreen.target);
    
    // If EQ window is open, show the EQ parameter table instead
    if (g_eq_window_open) {
        render_eq_info_panel();
        return;
    }
    
    // If network config window is open on top screen, render it
    if (g_net_config_open) {
        render_net_config_window();
        return;
    }
    
    // If options window is open on bottom screen, top screen shows normal info
    // (options renders itself on bottom screen)
    
    if (g_app_mode == APP_MODE_MANAGER) {
        // Manager mode: render show info panel on top screen
        render_show_info_panel();
        return;
    }
    
    // Creating new show - show input prompt on top screen
    if (g_creating_new_show) {
        // ===== SIMPLE INPUT DISPLAY =====
        C2D_DrawRectSolid(0, 0, 0.5f, SCREEN_WIDTH_TOP, 50, C2D_Color32(0x0F, 0x0F, 0x3F, 0xFF));
        C2D_DrawRectangle(0, 0, 0.5f, SCREEN_WIDTH_TOP, 50, CLR_BORDER, CLR_BORDER, CLR_BORDER, CLR_BORDER);
        draw_debug_text(&g_topScreen, "Enter Show Name", 15.0f, 10.0f, 0.55f, CLR_YELLOW);
        draw_debug_text(&g_topScreen, g_new_show_name, 15.0f, 30.0f, 0.50f, CLR_CYAN);
        
        // Rest of top screen empty
        C2D_DrawRectSolid(0, 50, 0.5f, SCREEN_WIDTH_TOP, SCREEN_HEIGHT_TOP - 50, CLR_BG_DARK);
    } else {
        // ===== TITLE BAR =====
        C2D_DrawRectSolid(0, 0, 0.5f, SCREEN_WIDTH_TOP, 35, C2D_Color32(0x0F, 0x0F, 0x3F, 0xFF));
        C2D_DrawRectangle(0, 0, 0.5f, SCREEN_WIDTH_TOP, 35, CLR_BORDER, CLR_BORDER, CLR_BORDER, CLR_BORDER);
        draw_debug_text(&g_topScreen, g_current_show.name, 15.0f, 8.0f, 0.55f, CLR_YELLOW);
        
        // Show step count
        char step_count_str[32];
        snprintf(step_count_str, sizeof(step_count_str), "Steps: %d", g_current_show.num_steps);
        draw_debug_text(&g_topScreen, step_count_str, SCREEN_WIDTH_TOP - 150, 8.0f, 0.45f, CLR_WHITE);
        
        // ===== STEPS LISTBOX =====
        // Listbox background
        C2D_DrawRectSolid(0, 35, 0.5f, SCREEN_WIDTH_TOP, 160, CLR_BG_MID);
        C2D_DrawRectangle(0, 35, 0.5f, SCREEN_WIDTH_TOP, 160, CLR_BORDER, CLR_BORDER, CLR_BORDER, CLR_BORDER);
        
        // Draw steps (max 8 visible at a time)
        int start_idx = g_selected_step - 3;  // Show selected step + context
        if (start_idx < 0) start_idx = 0;
        if (start_idx + 8 > g_current_show.num_steps) {
            start_idx = g_current_show.num_steps - 8;
            if (start_idx < 0) start_idx = 0;
        }
        
        float list_y = 42.0f;
        for (int i = start_idx; i < start_idx + 8 && i < g_current_show.num_steps; i++) {
            u32 step_color = CLR_WHITE;
            
            // Highlight selected step
            if (i == g_selected_step) {
                C2D_DrawRectSolid(5, list_y - 2, 0.5f, SCREEN_WIDTH_TOP - 10, 20, C2D_Color32(0x00, 0x44, 0x88, 0xFF));
                step_color = CLR_YELLOW;
            }
            
            char step_text[80];
            snprintf(step_text, sizeof(step_text), "[%3d] %-25s", i + 1, g_current_show.steps[i].name);
            draw_debug_text(&g_topScreen, step_text, 15.0f, list_y, 0.40f, step_color);
            
            list_y += 20.0f;
        }
        
        // ===== BOTTOM INFO BOXES =====
        // Left box - Messages
        C2D_DrawRectSolid(0, 195, 0.5f, 200, 45, CLR_BG_MID);
        C2D_DrawRectangle(0, 195, 0.5f, 200, 45, CLR_BORDER, CLR_BORDER, CLR_BORDER, CLR_BORDER);
        draw_debug_text(&g_topScreen, "Message:", 8.0f, 198.0f, 0.35f, CLR_CYAN);
        
        if (g_save_status_timer > 0) {
            u32 msg_color = CLR_WHITE;
            if (strstr(g_save_status, "ERROR")) {
                msg_color = CLR_RED;
            } else if (strstr(g_save_status, "SAVED")) {
                msg_color = CLR_GREEN;
            } else if (strstr(g_save_status, "Added") || strstr(g_save_status, "Duplicated")) {
                msg_color = CLR_YELLOW;
            } else if (strstr(g_save_status, "OK")) {
                msg_color = CLR_GREEN;
            }
            draw_debug_text(&g_topScreen, g_save_status, 8.0f, 213.0f, 0.35f, msg_color);
            g_save_status_timer--;
        } else {
            draw_debug_text(&g_topScreen, "Ready", 8.0f, 213.0f, 0.35f, CLR_WHITE);
        }
        
        // Right box - Debug info
        C2D_DrawRectSolid(200, 195, 0.5f, 200, 45, CLR_BG_MID);
        C2D_DrawRectangle(200, 195, 0.5f, 200, 45, CLR_BORDER, CLR_BORDER, CLR_BORDER, CLR_BORDER);
        draw_debug_text(&g_topScreen, "Info:", 208.0f, 198.0f, 0.35f, CLR_CYAN);
        
        char info_str[80];
        snprintf(info_str, sizeof(info_str), "Step %d/%d | Vol: %d%%", 
                 g_selected_step + 1, g_current_show.num_steps,
                 (int)(g_faders[0].value * 100));
        draw_debug_text(&g_topScreen, info_str, 208.0f, 213.0f, 0.35f, CLR_WHITE);
    }
}

void render_bot_screen(void)
{
    // Safety: Don't render if not fully initialized
    if (!g_init_complete) {
        C2D_TargetClear(g_botScreen.target, CLR_BG_DARK);
        C2D_SceneBegin(g_botScreen.target);
        return;
    }
    
    if (g_creating_new_show) {
        render_keyboard();
        return;
    }
    
    // If EQ window is open, render EQ on bottom screen instead of mixer
    if (g_eq_window_open) {
        render_eq_window();
        return;
    }
    
    // Normal mixer rendering
    
    u32 clrBg = C2D_Color32(0x1A, 0x1A, 0x1A, 0xFF);
    C2D_TargetClear(g_botScreen.target, clrBg);
    C2D_SceneBegin(g_botScreen.target);
    
    // Debug: Show image loading status
    char debug_msg[128];
    snprintf(debug_msg, sizeof(debug_msg), "RomFS:%d Grip:%d Fader:%d", 
             g_romfs_mounted, g_grip_loaded, g_fader_loaded);
    draw_debug_text(&g_botScreen, debug_msg, 5, 5, 0.30f, C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF));
    
    u32 clrMuteMain = C2D_Color32(0xDD, 0x33, 0x33, 0xFF);
    u32 clrMuteLight = C2D_Color32(0xFF, 0x66, 0x66, 0xFF);
    u32 clrMuteDark = C2D_Color32(0x88, 0x00, 0x00, 0xFF);
    u32 clrEqLight = C2D_Color32(0x66, 0xFF, 0x66, 0xFF);
    u32 clrEqDark = C2D_Color32(0x00, 0x88, 0x00, 0xFF);
    u32 clrText = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    u32 clrBorder = C2D_Color32(0x40, 0x40, 0x40, 0xFF);
    u32 clrGripMain = C2D_Color32(0xCC, 0xCC, 0xCC, 0xFF);  // Light gray grip
    u32 clrGripLight = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);  // White highlight
    u32 clrGripDark = C2D_Color32(0x66, 0x66, 0x66, 0xFF);   // Dark gray shadow
    u32 clrFaderTrack = C2D_Color32(0x1A, 0x1A, 0x1A, 0xFF);  // Very dark track
    u32 clrTickMark = C2D_Color32(0x55, 0x55, 0x55, 0xFF);
    
    for (int i = 0; i < NUM_FADERS; i++) {
        Fader *f = &g_faders[i];
        
        // Channel border
        C2D_DrawRectangle(f->x, f->y, 0.5f, f->w, f->h, clrBorder, clrBorder, clrBorder, clrBorder);
        
        // EQ button at top - color based on channel EQ enabled/disabled state
        // Get current EQ state from selected step
        ChannelEQ *eq = &g_current_show.steps[g_selected_step].eqs[i];
        u32 eq_color_main = eq->enabled ? clrEqLight : clrEqDark;
        u32 eq_color_light = eq->enabled ? C2D_Color32(0x88, 0xFF, 0x88, 0xFF) : C2D_Color32(0x44, 0x66, 0x44, 0xFF);
        u32 eq_color_dark = eq->enabled ? clrEqDark : C2D_Color32(0x00, 0x44, 0x00, 0xFF);
        draw_3d_button(f->x + 1, 5, f->w - 2, 14, eq_color_main, eq_color_light, eq_color_dark, 0);
        draw_debug_text(&g_botScreen, "Eq", f->x + 2, 7, 0.4f, clrText);
        
        // Channel number (below EQ button) - LARGER and CENTERED
        char label[4];
        snprintf(label, sizeof(label), "%d", f->id);
        draw_debug_text(&g_botScreen, label, f->x + f->w / 2 - 4, 20, 0.4f, clrText);
        
        // Volume in dB (calibrated scale) - LARGER and CENTERED
        char vol_str[8];
        float db = fader_value_to_db(f->value);
        if (db <= -100.0f) {
            snprintf(vol_str, sizeof(vol_str), "-inf");
        } else {
            snprintf(vol_str, sizeof(vol_str), "%.0f", db);
        }
        draw_debug_text(&g_botScreen, vol_str, f->x + f->w / 2 - 5, 30, 0.35f, clrText);
        
        // ===== FADER TRACK WITH SCALE MARKS =====
        float fader_top = 45;
        float fader_bottom = 205;
        float fader_height = fader_bottom - fader_top;
        float bar_x = f->x + (f->w - FADER_BAR_WIDTH) / 2;
        
        // Draw fader background image if loaded, otherwise use procedural fallback
        if (g_fader_bkg.tex != NULL) {
            // Draw fader background image scaled to fit the fader area
            // Image is 92x391, scale width to 20px (stretched horizontally for better visibility)
            float img_scale_x = 20.0f / 92.0f;  // Stretched for wider appearance
            float img_scale_y = fader_height / 391.0f;
            C2D_DrawImageAt(g_fader_bkg,
                           bar_x - 2 + (4 - 20) * 0.5f, fader_top,  // x, y (centered on 20px width)
                           0.5f, NULL, img_scale_x, img_scale_y);
        } else {
            // Fallback: procedural fader track
            C2D_DrawRectSolid(bar_x - 2, fader_top, 0.5f, FADER_BAR_WIDTH + 4, fader_height, clrFaderTrack);
            C2D_DrawRectangle(bar_x - 2, fader_top, 0.5f, FADER_BAR_WIDTH + 4, fader_height, 
                             clrBorder, clrBorder, clrBorder, clrBorder);
            
            // Draw tick marks
            const float tick_positions[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
            for (int tick = 0; tick < 5; tick++) {
                float tick_y = fader_top + (fader_height * tick_positions[tick]);
                C2D_DrawRectSolid(bar_x - 2, tick_y, 0.51f, 3, 1, clrTickMark);
                for (int minor = 1; minor < 4; minor++) {
                    float minor_tick_y = tick_y + (fader_height * 0.25f * (minor / 4.0f));
                    if (minor_tick_y < fader_bottom) {
                        C2D_DrawRectSolid(bar_x - 1, minor_tick_y, 0.51f, 2, 1, C2D_Color32(0x33, 0x33, 0x33, 0xFF));
                    }
                }
            }
        }
        
        // ===== GRIP/SLIDER =====
        // Grip corsa ridotta: da 15% (valore 0) a top (valore max)
        float grip_y = fader_bottom - (fader_height * (0.15f + f->value * 0.85f));
        float grip_x = f->x + (f->w - 11) / 2;  // Center the grip
        
        // Draw grip image if loaded, otherwise use procedural fallback
        if (g_grip_img.tex != NULL) {
            // Draw grip image scaled to appropriate size
            // Original is 38x68, we want approximately 12x16
            float grip_scale_x = 12.0f / 38.0f;
            float grip_scale_y = 16.0f / 68.0f;
            C2D_DrawImageAt(g_grip_img,
                           grip_x - (12 - 11) * 0.5f, grip_y,  // Center grip over track
                           0.5f, NULL, grip_scale_x, grip_scale_y);
        } else {
            // Fallback: procedural grip
            float grip_w = 11;
            float grip_h = 16;
            
            C2D_DrawRectSolid(grip_x + 1, grip_y + 1, 0.4f, grip_w, grip_h, C2D_Color32(0x00, 0x00, 0x00, 0x90));
            C2D_DrawRectSolid(grip_x, grip_y, 0.5f, grip_w, grip_h, clrGripMain);
            C2D_DrawRectSolid(grip_x + 1, grip_y + 1, 0.51f, grip_w - 2, 2, clrGripLight);
            C2D_DrawRectSolid(grip_x, grip_y + 2, 0.51f, 1, grip_h - 4, clrGripLight);
            C2D_DrawRectSolid(grip_x + grip_w - 1, grip_y + 2, 0.51f, 1, grip_h - 4, clrGripDark);
            C2D_DrawRectSolid(grip_x + grip_w / 2, grip_y + 3, 0.52f, 1, grip_h - 6, C2D_Color32(0xAA, 0xAA, 0xAA, 0xFF));
            C2D_DrawRectangle(grip_x, grip_y, 0.52f, grip_w, grip_h, clrGripDark, clrGripDark, clrGripDark, clrGripDark);
        }
        
        // Mute button at bottom (NEW POSITION)
        // Dark when not muted (sollevato), bright when muted (acceso/premuto)
        u32 mute_color_main = f->muted ? clrMuteMain : clrMuteDark;
        u32 mute_color_light = f->muted ? clrMuteLight : C2D_Color32(0x55, 0x00, 0x00, 0xFF);
        u32 mute_color_dark = f->muted ? clrMuteDark : C2D_Color32(0x44, 0x00, 0x00, 0xFF);
        draw_3d_button(f->x + 1, 210, f->w - 2, 17, mute_color_main, mute_color_light, mute_color_dark, f->muted);
        draw_debug_text(&g_botScreen, "M", f->x + 4, 212, 0.4f, clrText);
    }
}

void render_frame(void)
{
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    render_top_screen();
    
    if (g_app_mode == APP_MODE_MIXER) {
        render_bot_screen();  // Always show mixer on bottom screen
    } else if (g_options_window_open) {
        // Options window is MODAL - render ONLY this, nothing else on bottom screen
        render_options_window();
    } else {
        // Normal show manager mode
        render_show_manager();
        // Render network config window if open
        if (g_net_config_open) {
            render_net_config_window();
        }
    }
    
    C3D_FrameEnd(0);
}
