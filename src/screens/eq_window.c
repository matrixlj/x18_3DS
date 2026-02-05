#include "eq_window.h"
#include "../core/constants.h"

// Forward declarations
void fill_rect(int x1, int y1, int x2, int y2, unsigned short color, int screen);
void draw_pixel(int x, int y, unsigned short color, int screen);

// Get EQ type string representation
const char *eq_get_type_string(int type) {
    switch (type) {
        case EQ_TYPE_LCUT: return "LCut";
        case EQ_TYPE_LSHV: return "LShv";
        case EQ_TYPE_PEQ:  return "PEQ";
        case EQ_TYPE_VEQ:  return "VEQ";
        case EQ_TYPE_HSHV: return "HShv";
        case EQ_TYPE_HCUT: return "HCut";
        default: return "???";
    }
}

// Get parameter value from band
int eq_get_param_value(EQBandState *band, int param) {
    if (!band) return 0;
    
    switch (param) {
        case 0: return band->type;
        case 1: return band->frequency;
        case 2: return band->gain;
        case 3: return band->q;
        default: return 0;
    }
}

// Set parameter value in band
void eq_set_param_value(EQBandState *band, int param, int value) {
    if (!band) return;
    
    switch (param) {
        case 0: // Type (0-5)
            if (value < 0) value = 0;
            if (value > 5) value = 5;
            band->type = value;
            break;
        
        case 1: // Frequency (20-20000)
            if (value < 20) value = 20;
            if (value > 20000) value = 20000;
            band->frequency = value;
            break;
        
        case 2: // Gain (-150 to +150, represents -15.0 to +15.0 dB)
            if (value < -150) value = -150;
            if (value > 150) value = 150;
            band->gain = value;
            break;
        
        case 3: // Q (3-100, represents 0.3-10.0)
            if (value < 3) value = 3;
            if (value > 100) value = 100;
            band->q = value;
            break;
    }
}

// Main EQ window rendering
void eq_window_render_screen(AppState_Global *state) {
    if (!state) return;
    
    // Draw semi-transparent overlay on bottom screen
    // (In real implementation, would use transparency)
    for (int y = 0; y < SCREEN_HEIGHT_BOTTOM; y++) {
        for (int x = 0; x < SCREEN_WIDTH_BOTTOM; x++) {
            // Darken the background
            draw_pixel(x, y, COLOR_DARK_GRAY, 1);
        }
    }
    
    // Draw EQ window background
    fill_rect(EQ_WINDOW_X, EQ_WINDOW_Y, 
              EQ_WINDOW_X + EQ_WINDOW_WIDTH, 
              EQ_WINDOW_Y + EQ_WINDOW_HEIGHT, 
              COLOR_DARK_GRAY, 1);
    
    // Draw window border
    fill_rect(EQ_WINDOW_X, EQ_WINDOW_Y, 
              EQ_WINDOW_X + EQ_WINDOW_WIDTH, EQ_WINDOW_Y + 1, 
              COLOR_CYAN, 1);
    fill_rect(EQ_WINDOW_X, EQ_WINDOW_Y, 
              EQ_WINDOW_X + 1, EQ_WINDOW_Y + EQ_WINDOW_HEIGHT, 
              COLOR_CYAN, 1);
    fill_rect(EQ_WINDOW_X + EQ_WINDOW_WIDTH - 1, EQ_WINDOW_Y, 
              EQ_WINDOW_X + EQ_WINDOW_WIDTH, EQ_WINDOW_Y + EQ_WINDOW_HEIGHT, 
              COLOR_CYAN, 1);
    fill_rect(EQ_WINDOW_X, EQ_WINDOW_Y + EQ_WINDOW_HEIGHT - 1, 
              EQ_WINDOW_X + EQ_WINDOW_WIDTH, EQ_WINDOW_Y + EQ_WINDOW_HEIGHT, 
              COLOR_CYAN, 1);
    
    // Draw header
    eq_window_render_header(state);
    
    // Draw band list
    eq_window_render_bands(state);
    
    // Draw parameter editor
    eq_window_render_param_editor(state);
}

// Render header with channel name
void eq_window_render_header(AppState_Global *state) {
    if (!state) return;
    
    int header_y = EQ_WINDOW_Y + 5;
    
    // Channel header: "CH X EQ"
    // (Simple placeholder rendering)
    fill_rect(EQ_WINDOW_X + 5, header_y, EQ_WINDOW_X + 100, header_y + 10, 
              COLOR_DARK_GRAY, 1);
}

// Render EQ bands list
void eq_window_render_bands(AppState_Global *state) {
    if (!state) return;
    
    int y_start = EQ_WINDOW_Y + 20;
    
    for (int i = 0; i < 5; i++) {
        int is_selected = (state->eq_window.selected_band == i) ? 1 : 0;
        
        eq_window_render_band_item(y_start + (i * EQ_BAND_ITEM_HEIGHT), 
                                    i, 
                                    &state->eq_window.bands[i], 
                                    is_selected);
    }
}

// Render single band item
void eq_window_render_band_item(int y, int band_id, EQBandState *band, int selected) {
    unsigned short bg_color = selected ? COLOR_DARK_GRAY : COLOR_BLACK;
    unsigned short text_color = selected ? COLOR_CYAN : COLOR_WHITE;
    
    int x = EQ_WINDOW_X + 5;
    int width = EQ_WINDOW_WIDTH - 10;
    
    // Draw background
    fill_rect(x, y, x + width, y + EQ_BAND_ITEM_HEIGHT - 2, bg_color, 1);
    
    // Draw selection border if selected
    if (selected) {
        fill_rect(x, y, x + width, y + 1, COLOR_CYAN, 1);
        fill_rect(x, y, x + 1, y + EQ_BAND_ITEM_HEIGHT - 2, COLOR_CYAN, 1);
    }
    
    // Draw band number and type (simple pixel representation)
    // Band 1-5 indicator
    draw_pixel(x + 5, y + 5, text_color, 1);
    
    // Type indicator (would show type string in real version)
    for (int i = 0; i < 10; i++) {
        if ((i % 3) == 0) {
            draw_pixel(x + 20 + i, y + 5, text_color, 1);
        }
    }
}

// Render parameter editor
void eq_window_render_param_editor(AppState_Global *state) {
    if (!state) return;
    
    // Show controls for editing selected band/parameter
    // D-Pad: navigate
    // A/B: confirm/cancel
    // X/Y: adjust value
    
    int editor_y = EQ_WINDOW_Y + EQ_WINDOW_HEIGHT - 30;
    
    // Draw parameter value display area
    fill_rect(EQ_WINDOW_X + 5, editor_y, 
              EQ_WINDOW_X + EQ_WINDOW_WIDTH - 5, 
              editor_y + 25, 
              COLOR_DARK_GRAY, 1);
}

// Handle button input
void eq_window_handle_button_input(AppState_Global *state, int button) {
    if (!state) return;
    
    switch (button) {
        case 0: // D-Pad Left
            // Move to previous band
            if (state->eq_window.selected_band > 0) {
                state->eq_window.selected_band--;
            }
            break;
        
        case 1: // D-Pad Right
            // Move to next band
            if (state->eq_window.selected_band < 4) {
                state->eq_window.selected_band++;
            }
            break;
        
        case 2: // D-Pad Up
            // Adjust parameter value up
            eq_window_adjust_parameter(state, 5);
            break;
        
        case 3: // D-Pad Down
            // Adjust parameter value down
            eq_window_adjust_parameter(state, -5);
            break;
        
        case 4: // A button - Cycle through band types
            eq_window_cycle_band_type(state);
            break;
        
        case 5: // B button - Close EQ window, return to mixer
            state->current_state = STATE_MIXER_VIEW;
            break;
        
        case 6: // X button - Switch to different parameter
            state->eq_window.selected_param++;
            if (state->eq_window.selected_param > 3) {
                state->eq_window.selected_param = 0;
            }
            break;
        
        case 7: // Y button - Reset band to default
            // Reset selected band
            state->eq_window.bands[state->eq_window.selected_band].type = EQ_TYPE_PEQ;
            state->eq_window.bands[state->eq_window.selected_band].frequency = 1000;
            state->eq_window.bands[state->eq_window.selected_band].gain = 0;
            state->eq_window.bands[state->eq_window.selected_band].q = 50;  // 5.0
            break;
    }
}

// Adjust selected parameter
void eq_window_adjust_parameter(AppState_Global *state, int delta) {
    if (!state) return;
    
    int band_idx = state->eq_window.selected_band;
    int param_idx = state->eq_window.selected_param;
    
    EQBandState *band = &state->eq_window.bands[band_idx];
    int current_value = eq_get_param_value(band, param_idx);
    
    eq_set_param_value(band, param_idx, current_value + delta);
}

// Cycle through band types
void eq_window_cycle_band_type(AppState_Global *state) {
    if (!state) return;
    
    int band_idx = state->eq_window.selected_band;
    EQBandState *band = &state->eq_window.bands[band_idx];
    
    band->type++;
    if (band->type > EQ_TYPE_HCUT) {
        band->type = EQ_TYPE_LCUT;
    }
}
