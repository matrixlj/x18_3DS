#include "mixer_screen.h"
#include "../core/constants.h"

// Forward declarations from main.c
void fill_rect(int x1, int y1, int x2, int y2, unsigned short color, int screen);
void draw_pixel(int x, int y, unsigned short color, int screen);

// Get X position for channel
int mixer_get_channel_x(int channel_id) {
    return MIXER_TOP_MARGIN + (channel_id * (MIXER_CHANNEL_WIDTH + MIXER_CHANNEL_SPACING));
}

// Main mixer screen rendering
void mixer_render_screen(AppState_Global *state) {
    // Clear bottom screen with dark background
    fill_rect(0, 0, SCREEN_WIDTH_BOTTOM - 1, SCREEN_HEIGHT_BOTTOM - 1, COLOR_DARK_GRAY, 1);
    
    // Draw title bar
    fill_rect(0, 0, SCREEN_WIDTH_BOTTOM - 1, 20, COLOR_BLACK, 1);
    
    // Draw channel info (simple placeholder for now - needs proper text rendering)
    // Shows "Step: 1" and mixer IP
    fill_rect(5, 3, 100, 18, COLOR_BLACK, 1);
    
    // Draw all 16 channels
    for (int i = 0; i < NUM_CHANNELS; i++) {
        int is_selected = (state->selected_channel == i) ? 1 : 0;
        mixer_render_channel(i, &state->channels[i], mixer_get_channel_x(i), is_selected);
    }
    
    // Draw bottom status bar
    fill_rect(0, SCREEN_HEIGHT_BOTTOM - 15, SCREEN_WIDTH_BOTTOM - 1, SCREEN_HEIGHT_BOTTOM - 1, COLOR_BLACK, 1);
    
    // Status info: "IP: 192.168.1.100 | Ch: X | Step: Y"
    // Placeholder - would need proper text rendering
}

// Render single channel widget (fader + buttons)
void mixer_render_channel(int channel_id, ChannelState *state, int x, int selected) {
    unsigned short border_color = selected ? COLOR_CYAN : COLOR_GRAY;
    unsigned short bg_color = selected ? COLOR_DARK_GRAY : COLOR_BLACK;
    
    // Draw channel background
    fill_rect(x, MIXER_FADER_TOP - 5, x + MIXER_CHANNEL_WIDTH - 1, 
              MIXER_EQ_TOP + MIXER_BUTTON_SIZE + 5, bg_color, 1);
    
    // Draw channel border if selected
    if (selected) {
        // Top border
        fill_rect(x, MIXER_FADER_TOP - 5, x + MIXER_CHANNEL_WIDTH - 1, MIXER_FADER_TOP - 4, border_color, 1);
        // Left border
        fill_rect(x, MIXER_FADER_TOP - 5, x + 1, MIXER_EQ_TOP + MIXER_BUTTON_SIZE + 5, border_color, 1);
        // Right border
        fill_rect(x + MIXER_CHANNEL_WIDTH - 2, MIXER_FADER_TOP - 5, x + MIXER_CHANNEL_WIDTH - 1, 
                  MIXER_EQ_TOP + MIXER_BUTTON_SIZE + 5, border_color, 1);
    }
    
    // Draw fader
    mixer_render_fader(x + 2, MIXER_FADER_TOP, state->fader_level, COLOR_GREEN);
    
    // Draw mute button
    unsigned short mute_color = state->mute ? COLOR_RED : COLOR_GRAY;
    mixer_render_button(x + 3, MIXER_MUTE_TOP, MIXER_BUTTON_SIZE, 
                       state->mute ? "M" : " ", state->mute, mute_color);
    
    // Draw EQ button
    unsigned short eq_color = COLOR_YELLOW;
    mixer_render_button(x + 3, MIXER_EQ_TOP, MIXER_BUTTON_SIZE, 
                       "E", 0, eq_color);
    
    // Draw channel number (simple vertical text representation)
    // Would use proper font rendering here
    int channel_num = channel_id + 1;
    if (channel_num < 10) {
        // Draw digit as simple pattern
        draw_pixel(x + 5, MIXER_FADER_TOP - 10, COLOR_WHITE, 1);
    }
}

// Render vertical fader
void mixer_render_fader(int x, int y, int level, unsigned short color) {
    // level is 0-100
    if (level < 0) level = 0;
    if (level > 100) level = 100;
    
    unsigned short bg_color = COLOR_DARK_GRAY;
    
    // Draw fader track background
    fill_rect(x, y, x + 10, y + MIXER_FADER_HEIGHT, bg_color, 1);
    
    // Draw fader track border
    fill_rect(x, y, x + 10, y + 1, COLOR_GRAY, 1);
    fill_rect(x, y, x + 1, y + MIXER_FADER_HEIGHT, COLOR_GRAY, 1);
    fill_rect(x + 9, y, x + 10, y + MIXER_FADER_HEIGHT, COLOR_GRAY, 1);
    fill_rect(x, y + MIXER_FADER_HEIGHT - 1, x + 10, y + MIXER_FADER_HEIGHT, COLOR_GRAY, 1);
    
    // Calculate fader position (bottom-aligned, so 0 = bottom, 100 = top)
    // fader_pos = (100 - level) * MIXER_FADER_HEIGHT / 100
    int fader_pos = ((100 - level) * MIXER_FADER_HEIGHT) / 100;
    if (fader_pos < 0) fader_pos = 0;
    if (fader_pos > MIXER_FADER_HEIGHT) fader_pos = MIXER_FADER_HEIGHT;
    
    // Draw fader thumb (colored square)
    int thumb_height = 8;
    int thumb_y = y + fader_pos;
    if (thumb_y + thumb_height > y + MIXER_FADER_HEIGHT) {
        thumb_y = y + MIXER_FADER_HEIGHT - thumb_height;
    }
    fill_rect(x + 1, thumb_y, x + 9, thumb_y + thumb_height, color, 1);
    
    // Draw thumb border
    fill_rect(x + 1, thumb_y, x + 9, thumb_y + 1, COLOR_WHITE, 1);
}

// Render button widget
void mixer_render_button(int x, int y, int size, const char *label, int pressed, unsigned short color) {
    unsigned short bg_color = pressed ? color : COLOR_DARK_GRAY;
    unsigned short border_color = pressed ? COLOR_WHITE : COLOR_GRAY;
    
    // Draw button background
    fill_rect(x, y, x + size, y + size, bg_color, 1);
    
    // Draw button border
    fill_rect(x, y, x + size, y + 1, border_color, 1);
    fill_rect(x, y, x + 1, y + size, border_color, 1);
    fill_rect(x + size - 1, y, x + size, y + size, border_color, 1);
    fill_rect(x, y + size - 1, x + size, y + size, border_color, 1);
    
    // Label would be drawn here (simple pixel patterns for now)
}

// Check if touch is on a channel element
// Returns 1 if on channel, 0 otherwise
// element: 0=fader, 1=mute, 2=eq
int mixer_is_touch_on_channel(int x, int y, int *channel_id, int *element) {
    // Check if touch is in fader/button area vertically
    if (y < MIXER_FADER_TOP || y > MIXER_EQ_TOP + MIXER_BUTTON_SIZE + 5) {
        return 0;
    }
    
    // Check which channel horizontally
    for (int i = 0; i < NUM_CHANNELS; i++) {
        int ch_x = mixer_get_channel_x(i);
        if (x >= ch_x && x <= ch_x + MIXER_CHANNEL_WIDTH) {
            *channel_id = i;
            
            // Determine which element
            if (y >= MIXER_FADER_TOP && y < MIXER_FADER_TOP + MIXER_FADER_HEIGHT) {
                *element = 0; // Fader
            } else if (y >= MIXER_MUTE_TOP && y < MIXER_MUTE_TOP + MIXER_BUTTON_SIZE) {
                *element = 1; // Mute button
            } else if (y >= MIXER_EQ_TOP && y < MIXER_EQ_TOP + MIXER_BUTTON_SIZE) {
                *element = 2; // EQ button
            } else {
                return 0;
            }
            
            return 1;
        }
    }
    
    return 0;
}

// Handle button input (D-Pad, A, B, X, Y)
void mixer_handle_button_input(AppState_Global *state, int button) {
    switch (button) {
        case 0: // D-Pad Left
            if (state->selected_channel > 0) {
                state->selected_channel--;
            }
            break;
        
        case 1: // D-Pad Right
            if (state->selected_channel < NUM_CHANNELS - 1) {
                state->selected_channel++;
            }
            break;
        
        case 2: // D-Pad Up
            // Adjust fader up
            mixer_adjust_fader(&state->channels[state->selected_channel], 5);
            break;
        
        case 3: // D-Pad Down
            // Adjust fader down
            mixer_adjust_fader(&state->channels[state->selected_channel], -5);
            break;
        
        case 4: // A button - Toggle mute on selected channel
            state->channels[state->selected_channel].mute = 
                !state->channels[state->selected_channel].mute;
            break;
        
        case 5: // B button - Exit to menu
            state->current_state = STATE_MENU;
            break;
        
        case 6: // X button - Open EQ window
            state->current_state = STATE_EQ_WINDOW;
            break;
        
        case 7: // Y button - Next step
            if (state->selected_step < 99) {
                state->selected_step++;
            }
            break;
    }
}

// Handle touch input on faders
void mixer_handle_touch_input(AppState_Global *state, int x, int y) {
    int channel_id;
    int element;
    
    if (mixer_is_touch_on_channel(x, y, &channel_id, &element)) {
        state->selected_channel = channel_id;
        
        if (element == 0) { // Touch on fader
            // Calculate fader level from Y position
            // y=MIXER_FADER_TOP means level=1.0 (full)
            // y=MIXER_FADER_TOP+MIXER_FADER_HEIGHT means level=0.0 (empty)
            int relative_y = y - MIXER_FADER_TOP;
            if (relative_y < 0) relative_y = 0;
            if (relative_y > MIXER_FADER_HEIGHT) relative_y = MIXER_FADER_HEIGHT;
            
            // new_level = 100 - (relative_y * 100 / MIXER_FADER_HEIGHT)
            int new_level = 100 - ((relative_y * 100) / MIXER_FADER_HEIGHT);
            state->channels[channel_id].fader_level = new_level;
            
        } else if (element == 1) { // Touch on mute button
            state->channels[channel_id].mute = !state->channels[channel_id].mute;
            
        } else if (element == 2) { // Touch on EQ button
            state->eq_window.channel_id = channel_id;
            state->current_state = STATE_EQ_WINDOW;
        }
    }
}

// Adjust fader level
void mixer_adjust_fader(ChannelState *channel, int delta) {
    // delta is already in 0-100 scale (5 = 5%)
    channel->fader_level += delta;
    
    // Clamp to 0 - 100
    if (channel->fader_level < 0) channel->fader_level = 0;
    if (channel->fader_level > 100) channel->fader_level = 100;
}
