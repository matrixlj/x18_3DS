#include "common.h"
#include "mixer_window.h"

// Helper function to get filter type name
const char* get_filter_type_name(EQFilterType type)
{
    switch(type) {
        case EQ_LCUT: return "LCut";
        case EQ_LSHV: return "Lshv";
        case EQ_PEQ:  return "PEQ";
        case EQ_VPEQ: return "VPEQ";
        case EQ_HSHV: return "HShv";
        case EQ_HCUT: return "HCut";
        default: return "???";
    }
}

// Convert fader value (0-1) to dB using calibration points
float fader_value_to_db(float value)
{
    // Calibration points from X18 specs
    // percentages: 0, 13, 26, 39, 51, 63, 76, 88, 100
    // db_values: -100, -50, -30, -20, -10, -5, 0, 5, 10
    
    float percent = value * 100.0f;
    
    // Calibration points
    float calibration_percent[] = {0, 13, 26, 39, 51, 63, 76, 88, 100};
    float calibration_db[] = {-100, -50, -30, -20, -10, -5, 0, 5, 10};
    
    // Find the range this percent falls into and interpolate
    for (int i = 0; i < 8; i++) {
        if (percent >= calibration_percent[i] && percent <= calibration_percent[i+1]) {
            // Linear interpolation between calibration points
            float t = (percent - calibration_percent[i]) / (calibration_percent[i+1] - calibration_percent[i]);
            float db = calibration_db[i] + t * (calibration_db[i+1] - calibration_db[i]);
            return db;
        }
    }
    
    // Default to max if > 100
    return 10.0f;
}

// Calculate EQ response at a specific frequency for a single band
float calculate_eq_response(EQBand *band, float freq_hz)
{
    if (band->frequency <= 0 || band->q_factor <= 0) return 0.0f;
    
    float delta = freq_hz / band->frequency;
    if (delta < 0.01f) delta = 0.01f;
    if (delta > 100.0f) delta = 100.0f;
    
    float octave_dist = logf(delta) / logf(2.0f);
    float response = 0.0f;
    
    switch(band->type) {
        case EQ_PEQ:  // Peak EQ - symmetric peak at center frequency
        {
            // Narrower gaussian based on Q factor
            float q_width = 1.0f / band->q_factor;
            response = band->gain * expf(-((octave_dist * octave_dist) / (2.0f * q_width * q_width)));
            break;
        }
        case EQ_VPEQ: // Vintage Peaky - sharper, more resonant peak
        {
            // Higher Q effect - narrower peak
            float q_width = 0.5f / band->q_factor;
            response = band->gain * expf(-((octave_dist * octave_dist) / (2.0f * q_width * q_width)));
            break;
        }
        case EQ_LSHV: // Low Shelf - boost/cut below frequency
        {
            // Smooth shelf response using tanh for smooth transition
            // Below frequency: full gain, above: gradually rolls off
            float transition_sharpness = band->q_factor * 2.0f;
            float shelf_curve = tanhf((1.0f - octave_dist) * transition_sharpness) * 0.5f + 0.5f;
            response = band->gain * shelf_curve;
            break;
        }
        case EQ_HSHV: // High Shelf - boost/cut above frequency
        {
            // Smooth shelf response using tanh for smooth transition
            // Above frequency: full gain, below: gradually rolls off
            float transition_sharpness = band->q_factor * 2.0f;
            float shelf_curve = tanhf((octave_dist - 1.0f) * transition_sharpness) * 0.5f + 0.5f;
            response = band->gain * shelf_curve;
            break;
        }
        case EQ_LCUT: // Low Cut (High Pass) - smooth high-pass filter
        {
            // Butterworth-style high pass: smooth rolloff below frequency
            // HP response = delta^order / (1 + delta^order)
            // delta < 1: HP ≈ 0 (blocks low freq)
            // delta > 1: HP ≈ 1 (passes high freq)
            float order = band->q_factor * 2.0f;  // Q determines steepness
            float hp_response = powf(delta, order) / (1.0f + powf(delta, order));
            // Gain controls the amount of cut/boost, follows gain at all points
            response = band->gain * (1.0f - hp_response);
            break;
        }
        case EQ_HCUT: // High Cut (Low Pass) - smooth low-pass filter
        {
            // Butterworth-style low pass: smooth rolloff above frequency
            // LP response = 1 / (1 + (f/f0)^order)
            // delta < 1: LP ≈ 1 (passes low freq)
            // delta > 1: LP ≈ 0 (blocks high freq)
            float order = band->q_factor * 2.0f;  // Q determines steepness
            float lp_response = 1.0f / (1.0f + powf(delta, order));
            // Gain controls the amount of cut/boost, follows gain at all points
            response = band->gain * (1.0f - lp_response);
            break;
        }
        default:
            response = 0.0f;
            break;
    }
    
    // Clamp response to reasonable values for visualization
    if (response > 15.0f) response = 15.0f;
    if (response < -15.0f) response = -15.0f;
    
    return response;
}

// Draw EQ curve and graph on bottom screen with touch controls
void render_eq_window(void)
{
    if (!g_eq_window_open) return;
    
    // Bounds checking
    if (g_selected_step < 0 || g_selected_step >= g_current_show.num_steps) {
        g_eq_window_open = 0;
        return;
    }
    if (g_eq_editing_channel < 0 || g_eq_editing_channel >= 16) {
        g_eq_window_open = 0;
        return;
    }
    
    u32 clrBg = C2D_Color32(0x1A, 0x1A, 0x1A, 0xFF);
    u32 clrBorder = C2D_Color32(0x50, 0x50, 0x50, 0xFF);
    u32 clrWhite = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    u32 clrCyan = C2D_Color32(0x00, 0xFF, 0xFF, 0xFF);
    u32 clrGreen = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);
    u32 clrYellow = C2D_Color32(0xFF, 0xFF, 0x00, 0xFF);
    u32 clrRed = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
    u32 clrOrange = C2D_Color32(0xFF, 0xA0, 0x00, 0xFF);
    u32 clrMagenta = C2D_Color32(0xFF, 0x00, 0xFF, 0xFF);
    
    C2D_TargetClear(g_botScreen.target, clrBg);
    C2D_SceneBegin(g_botScreen.target);
    
    ChannelEQ *eq = &g_current_show.steps[g_selected_step].eqs[g_eq_editing_channel];
    
    // ===== HEADER (0-18px) =====
    C2D_DrawRectSolid(0, 0, 0.5f, SCREEN_WIDTH_BOT, 18, C2D_Color32(0x0F, 0x0F, 0x3F, 0xFF));
    C2D_DrawRectangle(0, 0, 0.5f, SCREEN_WIDTH_BOT, 18, clrBorder, clrBorder, clrBorder, clrBorder);
    
    char header[64];
    snprintf(header, sizeof(header), "CH %02d", g_eq_editing_channel + 1);
    draw_debug_text(&g_botScreen, header, 8.0f, 3.0f, 0.45f, clrYellow);
    
    // Store enable button rect for touch detection (x=80, y=3, w=60, h=12)
    // Will be used in touch handler
    
    // Enable/Disable button (clickable zone)
    const char *enable_text = eq->enabled ? "ENABLED" : "DISABLED";
    u32 enable_color = eq->enabled ? clrGreen : clrRed;
    C2D_DrawRectSolid(80, 3, 0.5f, 60, 12, enable_color);
    C2D_DrawRectangle(80, 3, 0.5f, 60, 12, clrBorder, clrBorder, clrBorder, clrBorder);
    draw_debug_text(&g_botScreen, enable_text, 85.0f, 2.0f, 0.35f, clrWhite);
    
    // ===== EQ GRAPH AREA (20-100px) - Full width =====
    int graph_x = 0;
    int graph_y = 20;
    int graph_w = 320;  // Full screen width
    int graph_h = 80;
    
    C2D_DrawRectSolid(graph_x, graph_y, 0.5f, graph_w, graph_h, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
    C2D_DrawRectangle(graph_x, graph_y, 0.5f, graph_w, graph_h, clrBorder, clrBorder, clrBorder, clrBorder);
    
    // Draw frequency grid lines  
    float freq_markers[] = {20, 100, 1000, 10000, 20000};
    for (int i = 0; i < 5; i++) {
        float log_pos = (logf(freq_markers[i] / 20.0f) / logf(20000.0f / 20.0f));
        int x = graph_x + (int)(log_pos * graph_w);
        C2D_DrawRectSolid(x, graph_y, 0.5f, 1, graph_h, C2D_Color32(0x33, 0x33, 0x33, 0xFF));
    }
    
    // Draw center line (0dB)
    int center_y = graph_y + graph_h / 2;
    C2D_DrawRectSolid(graph_x, center_y, 0.5f, graph_w, 1, C2D_Color32(0x44, 0x44, 0x44, 0xFF));
    
    // Draw individual band curves - non-selected bands first
    u32 band_colors[] = {clrCyan, clrGreen, clrYellow, clrOrange, clrMagenta};
    
    // First pass: draw all non-selected bands
    for (int b = 0; b < 5; b++) {
        if (b == g_eq_selected_band) continue;  // Skip selected band, draw it last
        
        EQBand *band = &eq->bands[b];
        
        int prev_x = graph_x;
        int prev_y = center_y;
        
        for (int x = graph_x; x < graph_x + graph_w; x += 1) {
            float log_pos = (float)(x - graph_x) / (float)graph_w;
            float freq = 20.0f * powf(20000.0f / 20.0f, log_pos);
            
            float response_db = calculate_eq_response(band, freq);
            if (response_db > 15.0f) response_db = 15.0f;
            if (response_db < -15.0f) response_db = -15.0f;
            
            // Center the curve at y=0dB (center_y)
            int y = center_y - (int)(response_db * graph_h / 30.0f);
            
            if (x > graph_x) {
                // Draw thin line (1px) for each band
                C2D_DrawRectSolid(prev_x, prev_y, 0.51f, x - prev_x, 1, band_colors[b]);
            }
            
            prev_x = x;
            prev_y = y;
        }
    }
    
    // Second pass: draw selected band on top (foreground)
    {
        EQBand *band = &eq->bands[g_eq_selected_band];
        
        int prev_x = graph_x;
        int prev_y = center_y;
        
        for (int x = graph_x; x < graph_x + graph_w; x += 1) {
            float log_pos = (float)(x - graph_x) / (float)graph_w;
            float freq = 20.0f * powf(20000.0f / 20.0f, log_pos);
            
            float response_db = calculate_eq_response(band, freq);
            if (response_db > 15.0f) response_db = 15.0f;
            if (response_db < -15.0f) response_db = -15.0f;
            
            // Center the curve at y=0dB (center_y)
            int y = center_y - (int)(response_db * graph_h / 30.0f);
            
            if (x > graph_x) {
                // Draw selected band with 2px width for visibility
                C2D_DrawRectSolid(prev_x, prev_y - 1, 0.52f, x - prev_x, 3, band_colors[g_eq_selected_band]);
            }
            
            prev_x = x;
            prev_y = y;
        }
    }
    
    // Draw combined result curve (white, thickest)
    {
        int prev_x = graph_x;
        int prev_y = center_y;
        
        for (int x = graph_x; x < graph_x + graph_w; x += 1) {
            float log_pos = (float)(x - graph_x) / (float)graph_w;
            float freq = 20.0f * powf(20000.0f / 20.0f, log_pos);
            
            float total_gain = 0;
            for (int b = 0; b < 5; b++) {
                total_gain += calculate_eq_response(&eq->bands[b], freq);
            }
            
            if (total_gain > 15.0f) total_gain = 15.0f;
            if (total_gain < -15.0f) total_gain = -15.0f;
            
            // Center the curve at y=0dB (center_y)
            int y = center_y - (int)(total_gain * graph_h / 30.0f);
            
            if (x > graph_x) {
                C2D_DrawRectSolid(prev_x, prev_y, 0.52f, x - prev_x, 2, clrWhite);
            }
            
            prev_x = x;
            prev_y = y;
        }
    }
    
    // ===== Q FACTOR CONTROL (102-120px) =====
    EQBand *selected_band = &eq->bands[g_eq_selected_band];
    
    C2D_DrawRectSolid(0, 102, 0.5f, 320, 18, C2D_Color32(0x1F, 0x1F, 0x4F, 0xFF));
    C2D_DrawRectangle(0, 102, 0.5f, 320, 18, clrBorder, clrBorder, clrBorder, clrBorder);
    
    // Draw Q factor position indicator (vertical line showing current Q value)
    float q_norm = (10.0f - selected_band->q_factor) / 9.7f;  // Inverse mapping (0.3->1, 10->0)
    int q_indicator_x = (int)(q_norm * 320.0f);
    C2D_DrawRectSolid(q_indicator_x - 1, 103, 0.51f, 3, 16, clrRed);
    
    char q_str[64];
    snprintf(q_str, sizeof(q_str), "Q: %.2f", selected_band->q_factor);
    draw_debug_text(&g_botScreen, q_str, 15.0f, 106.0f, 0.4f, clrYellow);
    
    // Type selection buttons (122-145px) - 6 types: LCut, Lshv, PEQ, VPEQ, HShv, HCut
    C2D_DrawRectSolid(0, 122, 0.5f, SCREEN_WIDTH_BOT, 24, C2D_Color32(0x1A, 0x1A, 0x1A, 0xFF));
    C2D_DrawRectangle(0, 122, 0.5f, SCREEN_WIDTH_BOT, 24, clrBorder, clrBorder, clrBorder, clrBorder);
    
    const char *type_names[] = {"LCut", "Lshv", "PEQ", "VPEQ", "HShv", "HCut"};
    int button_height = 20;
    
    for (int t = 0; t < 6; t++) {
        int btn_x = (SCREEN_WIDTH_BOT * t) / 6;
        int btn_w = (SCREEN_WIDTH_BOT * (t + 1)) / 6 - btn_x;
        u32 btn_color = (t == selected_band->type) ? clrGreen : C2D_Color32(0x00, 0x22, 0x66, 0xFF);
        C2D_DrawRectSolid(btn_x, 125, 0.5f, btn_w - 1, button_height - 2, btn_color);
        C2D_DrawRectangle(btn_x, 125, 0.5f, btn_w - 1, button_height - 2, clrBorder, clrBorder, clrBorder, clrBorder);
        
        u32 txt_color = (t == selected_band->type) ? C2D_Color32(0x00, 0x00, 0x00, 0xFF) : clrCyan;
        draw_debug_text(&g_botScreen, type_names[t], btn_x + btn_w / 2.0f - 8.0f, 128.0f, 0.38f, txt_color);
    }
    
    // ===== BAND INFO PANEL (147-240px) =====
    int info_y = 147;
    int band_height = 18;  // 5 bands * 18px = 90px
    
    for (int b = 0; b < 5; b++) {
        EQBand *band = &eq->bands[b];
        int y = info_y + (b * band_height);
        
        // Highlight selected band
        u32 bg_color = (b == g_eq_selected_band) ? C2D_Color32(0x00, 0x44, 0x88, 0xFF) : C2D_Color32(0x25, 0x25, 0x25, 0xFF);
        C2D_DrawRectSolid(0, y, 0.5f, SCREEN_WIDTH_BOT, band_height - 1, bg_color);
        C2D_DrawRectangle(0, y, 0.5f, SCREEN_WIDTH_BOT, band_height - 1, clrBorder, clrBorder, clrBorder, clrBorder);
        
        const char *type_name = get_filter_type_name(band->type);
        char band_str[128];
        snprintf(band_str, sizeof(band_str), "B%d[%s]  %.0fHz  G:%+.1fdB  Q:%.1f",
                 b + 1, type_name, band->frequency, band->gain, band->q_factor);
        
        u32 text_color = (b == g_eq_selected_band) ? clrYellow : clrCyan;
        draw_debug_text(&g_botScreen, band_str, 5.0f, y + 2.0f, 0.36f, text_color);
    }
}

void handle_eq_input(u32 kDown, u32 kHeld)
{
    // Bounds checking - close window if indices become invalid
    if (g_selected_step < 0 || g_selected_step >= g_current_show.num_steps) {
        g_eq_window_open = 0;
        return;
    }
    if (g_eq_editing_channel < 0 || g_eq_editing_channel >= 16) {
        g_eq_window_open = 0;
        return;
    }
    
    // D-Pad: Navigate between bands and parameters
    if (kDown & KEY_DUP) {
        // Up: Move to previous band
        g_eq_selected_band--;
        if (g_eq_selected_band < 0) g_eq_selected_band = 4;
        g_eq_param_selected = 0;  // Reset to freq parameter
    }
    if (kDown & KEY_DDOWN) {
        // Down: Move to next band
        g_eq_selected_band++;
        if (g_eq_selected_band >= 5) g_eq_selected_band = 0;
        g_eq_param_selected = 0;  // Reset to freq parameter
    }
    if (kDown & KEY_DLEFT) {
        // Left: Previous parameter (freq <- gain <- q)
        g_eq_param_selected--;
        if (g_eq_param_selected < 0) g_eq_param_selected = 2;
    }
    if (kDown & KEY_DRIGHT) {
        // Right: Next parameter (freq -> gain -> q)
        g_eq_param_selected++;
        if (g_eq_param_selected >= 3) g_eq_param_selected = 0;
    }
    
    ChannelEQ *eq = &g_current_show.steps[g_selected_step].eqs[g_eq_editing_channel];
    EQBand *band = &eq->bands[g_eq_selected_band];
    
    // L button helper: modify parameters with D-Pad
    if (kHeld & KEY_L) {
        if (kDown & KEY_DUP) {
            // Increase parameter value
            if (g_eq_param_selected == 0) {
                // Frequency: increase by 1% or 50 Hz minimum
                band->frequency *= 1.05f;
                if (band->frequency > 20000.0f) band->frequency = 20000.0f;
            } else if (g_eq_param_selected == 1) {
                // Gain: increase by 0.5 dB
                band->gain += 0.5f;
                if (band->gain > 15.0f) band->gain = 15.0f;
            } else if (g_eq_param_selected == 2) {
                // Q: increase by 0.1
                band->q_factor += 0.1f;
                if (band->q_factor > 10.0f) band->q_factor = 10.0f;
            }
        }
        if (kDown & KEY_DDOWN) {
            // Decrease parameter value
            if (g_eq_param_selected == 0) {
                // Frequency: decrease by ~5%
                band->frequency /= 1.05f;
                if (band->frequency < 20.0f) band->frequency = 20.0f;
            } else if (g_eq_param_selected == 1) {
                // Gain: decrease by 0.5 dB
                band->gain -= 0.5f;
                if (band->gain < -15.0f) band->gain = -15.0f;
            } else if (g_eq_param_selected == 2) {
                // Q: decrease by 0.1
                band->q_factor -= 0.1f;
                if (band->q_factor < 0.3f) band->q_factor = 0.3f;
            }
        }
    }
    
    // A: Toggle EQ enabled/disabled for current channel
    if (kDown & KEY_A) {
        eq->enabled = 1 - eq->enabled;
    }
    
    // B: Close EQ window
    if (kDown & KEY_B) {
        g_eq_window_open = 0;
        extern void save_step_from_faders(int step_idx);
        extern void save_show_to_file(Show *show);
        save_step_from_faders(g_selected_step);
        save_show_to_file(&g_current_show);
    }
}
