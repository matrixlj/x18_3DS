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
    
    // Enable/Disable button (clickable zone)
    const char *enable_text = eq->enabled ? "ENABLED" : "DISABLED";
    u32 enable_color = eq->enabled ? clrGreen : clrRed;
    C2D_DrawRectSolid(80, 3, 0.5f, 60, 12, enable_color);
    C2D_DrawRectangle(80, 3, 0.5f, 60, 12, clrBorder, clrBorder, clrBorder, clrBorder);
    draw_debug_text(&g_botScreen, enable_text, 85.0f, 2.0f, 0.35f, clrWhite);
    
    // ===== TYPE SELECTION BUTTONS (20-36px) - 6 types: LCut, Lshv, PEQ, VPEQ, HShv, HCut =====
    EQBand *selected_band = &eq->bands[g_eq_selected_band];
    const char *type_names[] = {"LCut", "Lshv", "PEQ", "VPEQ", "HShv", "HCut"};
    int button_height = 16;
    
    for (int t = 0; t < 6; t++) {
        int btn_x = (SCREEN_WIDTH_BOT * t) / 6;
        int btn_w = (SCREEN_WIDTH_BOT * (t + 1)) / 6 - btn_x;
        u32 btn_color = (t == selected_band->type) ? clrGreen : C2D_Color32(0x00, 0x22, 0x66, 0xFF);
        C2D_DrawRectSolid(btn_x, 20, 0.5f, btn_w - 1, button_height - 1, btn_color);
        C2D_DrawRectangle(btn_x, 20, 0.5f, btn_w - 1, button_height - 1, clrBorder, clrBorder, clrBorder, clrBorder);
        
        u32 txt_color = (t == selected_band->type) ? C2D_Color32(0x00, 0x00, 0x00, 0xFF) : clrCyan;
        draw_debug_text(&g_botScreen, type_names[t], btn_x + btn_w / 2.0f - 7.0f, 20.5f, 0.35f, txt_color);
    }
    
    // ===== EQ GRAPH AREA (38-220px) - Full width, maximized =====
    int graph_x = 0;
    int graph_y = 38;
    int graph_w = 320;
    int graph_h = 182;  // Max height graph
    
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
    
    // Band colors
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
    
    // Third pass: draw combined result curve (white, thickest)
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
    
    // ===== DRAW BAND CIRCLES at (freq, gain) positions =====
    for (int b = 0; b < 5; b++) {
        EQBand *band = &eq->bands[b];
        
        // Calculate position on graph
        float log_pos = logf(band->frequency / 20.0f) / logf(20000.0f / 20.0f);
        int circle_x = graph_x + (int)(log_pos * graph_w);
        
        // Gain to Y position
        int circle_y = center_y - (int)(band->gain * graph_h / 30.0f);
        
        // Clamp to graph bounds
        if (circle_x < graph_x) circle_x = graph_x;
        if (circle_x > graph_x + graph_w) circle_x = graph_x + graph_w;
        if (circle_y < graph_y) circle_y = graph_y;
        if (circle_y > graph_y + graph_h) circle_y = graph_y + graph_h;
        
        // Draw circle (8px radius)
        int radius = 4;
        u32 circle_color = band_colors[b];
        
        // Draw filled circle using small rectangles
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx*dx + dy*dy <= radius*radius) {
                    C2D_DrawRectSolid(circle_x + dx, circle_y + dy, 0.52f, 1, 1, circle_color);
                }
            }
        }
        
        // Draw circle border for visibility
        C2D_DrawRectSolid(circle_x - radius, circle_y, 0.53f, radius * 2, 1, clrWhite);
        C2D_DrawRectSolid(circle_x, circle_y - radius, 0.53f, 1, radius * 2, clrWhite);
    }
    
    // ===== Q FACTOR CONTROL (220-240px) - Bottom of screen =====
    C2D_DrawRectSolid(0, 220, 0.5f, 320, 20, C2D_Color32(0x1F, 0x1F, 0x4F, 0xFF));
    C2D_DrawRectangle(0, 220, 0.5f, 320, 20, clrBorder, clrBorder, clrBorder, clrBorder);
    
    // Draw Q factor position indicator (horizontal fader bar)
    int q_bar_y = 220;
    int q_bar_x_start = 64;
    int q_bar_width = 192;
    
    // Draw the fader bar background
    C2D_DrawRectSolid(q_bar_x_start, q_bar_y + 4, 0.51f, q_bar_width, 12, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
    
    // Draw Q factor position indicator (fills from left to right)
    float q_norm = (10.0f - selected_band->q_factor) / 9.7f;  // Inverse mapping (0.3->1, 10->0)
    int q_fill_width = (int)(q_norm * q_bar_width);
    C2D_DrawRectSolid(q_bar_x_start, q_bar_y + 4, 0.51f, q_fill_width, 12, clrRed);
    
    char q_str[64];
    snprintf(q_str, sizeof(q_str), "Q: %.2f", selected_band->q_factor);
    draw_debug_text(&g_botScreen, q_str, 15.0f, 222.0f, 0.4f, clrYellow);
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

// Render EQ parameter table on top screen
void render_eq_info_panel(void)
{
    if (!g_eq_window_open || g_selected_step < 0 || g_eq_editing_channel < 0) return;
    
    ChannelEQ *eq = &g_current_show.steps[g_selected_step].eqs[g_eq_editing_channel];
    
    u32 clrBg = C2D_Color32(0x1A, 0x1A, 0x1A, 0xFF);
    u32 clrBorder = C2D_Color32(0x50, 0x50, 0x50, 0xFF);
    u32 clrYellow = C2D_Color32(0xFF, 0xFF, 0x00, 0xFF);
    u32 clrCyan = C2D_Color32(0x00, 0xFF, 0xFF, 0xFF);
    u32 clrBlue = C2D_Color32(0x00, 0x77, 0xFF, 0xFF);
    
    // Band colors (same as in graph)
    u32 band_colors[] = {
        C2D_Color32(0x00, 0xFF, 0xFF, 0xFF),  // Cyan
        C2D_Color32(0x00, 0xFF, 0x00, 0xFF),  // Green
        C2D_Color32(0xFF, 0xFF, 0x00, 0xFF),  // Yellow
        C2D_Color32(0xFF, 0xA0, 0x00, 0xFF),  // Orange
        C2D_Color32(0xFF, 0x00, 0xFF, 0xFF)   // Magenta
    };
    
    C2D_TargetClear(g_topScreen.target, clrBg);
    C2D_SceneBegin(g_topScreen.target);
    
    // ===== HEADER =====
    C2D_DrawRectSolid(0, 0, 0.5f, SCREEN_WIDTH_TOP, 22, C2D_Color32(0x0F, 0x0F, 0x3F, 0xFF));
    C2D_DrawRectangle(0, 0, 0.5f, SCREEN_WIDTH_TOP, 22, clrBorder, clrBorder, clrBorder, clrBorder);
    
    char header[64];
    snprintf(header, sizeof(header), "EQ Parameters - Channel %02d", g_eq_editing_channel + 1);
    draw_debug_text(&g_topScreen, header, 10.0f, 2.0f, 0.40f, clrYellow);
    
    // ===== TABLE SETUP =====
    int label_col_width = 25;  // Narrower label column
    int col_width = (SCREEN_WIDTH_TOP - label_col_width) / 5;  // Calculated for 5 bands
    int row_height = 44;
    int table_y = 25;
    
    // Rows: Frequency, Gain, Q, Type
    const char *row_labels[] = {"Freq", "Gain", "Q", "Type"};
    
    for (int row = 0; row < 4; row++) {
        int row_y = table_y + (row * row_height);
        
        // Draw row background
        C2D_DrawRectSolid(0, row_y, 0.5f, SCREEN_WIDTH_TOP, row_height - 1, clrBg);
        C2D_DrawRectangle(0, row_y, 0.5f, SCREEN_WIDTH_TOP, row_height - 1, clrBorder, clrBorder, clrBorder, clrBorder);
        
        // Draw row label
        draw_debug_text(&g_topScreen, row_labels[row], 5.0f, row_y + 18.0f, 0.40f, clrCyan);
        
        // Draw band values
        for (int b = 0; b < 5; b++) {
            int col_x = label_col_width + (b * col_width);
            
            // Highlight selected band
            if (b == g_eq_selected_band) {
                C2D_DrawRectSolid(col_x, row_y, 0.5f, col_width - 1, row_height - 1, 
                                  C2D_Color32(0x00, 0x44, 0x88, 0xFF));
            }
            
            char value_str[32] = {0};
            
            if (row == 0) {  // Frequency
                snprintf(value_str, sizeof(value_str), "%.0f", eq->bands[b].frequency);
                float text_x = col_x + col_width / 2.0f - 15.0f;
                draw_debug_text(&g_topScreen, value_str, text_x, row_y + 10.0f, 0.50f, band_colors[b]);
                draw_debug_text(&g_topScreen, "Hz", text_x + 22.0f, row_y + 18.0f, 0.32f, band_colors[b]);
            } 
            else if (row == 1) {  // Gain
                snprintf(value_str, sizeof(value_str), "%+.1f", eq->bands[b].gain);
                float text_x = col_x + col_width / 2.0f - 15.0f;
                draw_debug_text(&g_topScreen, value_str, text_x, row_y + 10.0f, 0.50f, band_colors[b]);
                draw_debug_text(&g_topScreen, "dB", text_x + 22.0f, row_y + 18.0f, 0.32f, band_colors[b]);
            }
            else if (row == 2) {  // Q
                snprintf(value_str, sizeof(value_str), "%.2f", eq->bands[b].q_factor);
                float text_x = col_x + col_width / 2.0f - 12.0f;
                draw_debug_text(&g_topScreen, value_str, text_x, row_y + 18.0f, 0.48f, band_colors[b]);
            }
            else if (row == 3) {  // Type
                snprintf(value_str, sizeof(value_str), "%s", get_filter_type_name(eq->bands[b].type));
                float text_x = col_x + col_width / 2.0f - 12.0f;
                draw_debug_text(&g_topScreen, value_str, text_x, row_y + 18.0f, 0.45f, band_colors[b]);
            }
        }
    }
    
    // ===== FOOTER WITH INSTRUCTIONS =====
    int footer_y = 200;
    C2D_DrawRectSolid(0, footer_y, 0.5f, SCREEN_WIDTH_TOP, 40, C2D_Color32(0x0F, 0x0F, 0x3F, 0xFF));
    C2D_DrawRectangle(0, footer_y, 0.5f, SCREEN_WIDTH_TOP, 40, clrBorder, clrBorder, clrBorder, clrBorder);
    
    draw_debug_text(&g_topScreen, "D-Pad: Select | L+D-Pad: Adjust | A: Toggle | B: Close", 5.0f, footer_y + 5.0f, 0.30f, clrBlue);
    draw_debug_text(&g_topScreen, "Touch band circles on bottom screen to adjust Freq/Gain", 5.0f, footer_y + 18.0f, 0.28f, clrCyan);
}
