#include "touch_input.h"
#include "../screens/mixer_screen.h"

// Touch layout constants (from mixer_screen.h)
#define TOUCH_MIXER_CHANNEL_WIDTH 20
#define TOUCH_MIXER_CHANNEL_SPACING 2
#define TOUCH_MIXER_FADER_HEIGHT 80
#define TOUCH_MIXER_BUTTON_SIZE 10
#define TOUCH_MIXER_TOP_MARGIN 10
#define TOUCH_MIXER_FADER_TOP 30
#define TOUCH_MIXER_MUTE_TOP (TOUCH_MIXER_FADER_TOP + TOUCH_MIXER_FADER_HEIGHT + 5)
#define TOUCH_MIXER_EQ_TOP (TOUCH_MIXER_MUTE_TOP + TOUCH_MIXER_BUTTON_SIZE + 3)

// Initialize touch input manager
void touch_input_init(TouchInputManager *manager) {
    if (!manager) return;
    
    manager->is_pressed = 0;
    manager->last_x = 0;
    manager->last_y = 0;
    manager->press_frame = 0;
    manager->current_frame = 0;
    manager->last_event.type = TOUCH_EVENT_NONE;
    manager->last_event.x = 0;
    manager->last_event.y = 0;
    manager->last_event.duration = 0;
}

// Validate touch coordinates
int touch_is_valid_coords(int x, int y) {
    return (x >= 0 && x < TOUCH_SCREEN_WIDTH && y >= 0 && y < TOUCH_SCREEN_HEIGHT);
}

// Update touch input state
void touch_input_update(TouchInputManager *manager, int new_x, int new_y, int is_pressed) {
    if (!manager) return;
    
    if (!touch_is_valid_coords(new_x, new_y)) {
        new_x = manager->last_x;
        new_y = manager->last_y;
    }
    
    manager->current_frame++;
    
    // State transition logic
    if (is_pressed && !manager->is_pressed) {
        // Just pressed (press event)
        manager->is_pressed = 1;
        manager->press_frame = manager->current_frame;
        manager->last_event.type = TOUCH_EVENT_PRESS;
        manager->last_event.duration = 0;
        
    } else if (is_pressed && manager->is_pressed) {
        // Still pressed (move event if coordinates changed)
        if (new_x != manager->last_x || new_y != manager->last_y) {
            manager->last_event.type = TOUCH_EVENT_MOVE;
        } else {
            manager->last_event.type = TOUCH_EVENT_NONE;
        }
        manager->last_event.duration = manager->current_frame - manager->press_frame;
        
    } else if (!is_pressed && manager->is_pressed) {
        // Just released (release event)
        manager->is_pressed = 0;
        manager->last_event.type = TOUCH_EVENT_RELEASE;
        manager->last_event.duration = manager->current_frame - manager->press_frame;
        
    } else {
        // Not pressed and wasn't pressed
        manager->last_event.type = TOUCH_EVENT_NONE;
    }
    
    manager->last_x = new_x;
    manager->last_y = new_y;
    manager->last_event.x = new_x;
    manager->last_event.y = new_y;
}

// Get last touch event
TouchEvent touch_input_get_event(TouchInputManager *manager) {
    TouchEvent event = {TOUCH_EVENT_NONE, 0, 0, 0};
    
    if (manager) {
        event = manager->last_event;
    }
    
    return event;
}

// Check if touch is on a fader
// Returns 1 if on fader, sets channel_id
int touch_is_on_fader(int x, int y, int *channel_id) {
    // Check if Y is in fader zone
    if (y < TOUCH_MIXER_FADER_TOP || y >= TOUCH_MIXER_FADER_TOP + TOUCH_MIXER_FADER_HEIGHT) {
        return 0;
    }
    
    // Check which channel by X coordinate
    // Each channel is TOUCH_MIXER_CHANNEL_WIDTH + TOUCH_MIXER_CHANNEL_SPACING pixels
    int channel_x = x - TOUCH_MIXER_TOP_MARGIN;
    if (channel_x < 0) return 0;
    
    int total_ch_width = TOUCH_MIXER_CHANNEL_WIDTH + TOUCH_MIXER_CHANNEL_SPACING;
    int ch = channel_x / total_ch_width;
    
    // Verify we're within a valid channel
    if (ch >= 16) return 0;
    
    // Verify we're within the actual channel widget (not in spacing)
    int ch_offset = channel_x % total_ch_width;
    if (ch_offset >= TOUCH_MIXER_CHANNEL_WIDTH) return 0;
    
    if (channel_id) *channel_id = ch;
    return 1;
}

// Check if touch is on mute button
int touch_is_on_mute_button(int x, int y, int *channel_id) {
    // Check if Y is in mute button zone
    if (y < TOUCH_MIXER_MUTE_TOP || y >= TOUCH_MIXER_MUTE_TOP + TOUCH_MIXER_BUTTON_SIZE) {
        return 0;
    }
    
    // Check which channel by X coordinate
    int channel_x = x - TOUCH_MIXER_TOP_MARGIN;
    if (channel_x < 0) return 0;
    
    int total_ch_width = TOUCH_MIXER_CHANNEL_WIDTH + TOUCH_MIXER_CHANNEL_SPACING;
    int ch = channel_x / total_ch_width;
    
    if (ch >= 16) return 0;
    
    int ch_offset = channel_x % total_ch_width;
    if (ch_offset >= TOUCH_MIXER_CHANNEL_WIDTH) return 0;
    
    if (channel_id) *channel_id = ch;
    return 1;
}

// Check if touch is on EQ button
int touch_is_on_eq_button(int x, int y, int *channel_id) {
    // Check if Y is in EQ button zone
    if (y < TOUCH_MIXER_EQ_TOP || y >= TOUCH_MIXER_EQ_TOP + TOUCH_MIXER_BUTTON_SIZE) {
        return 0;
    }
    
    // Check which channel by X coordinate
    int channel_x = x - TOUCH_MIXER_TOP_MARGIN;
    if (channel_x < 0) return 0;
    
    int total_ch_width = TOUCH_MIXER_CHANNEL_WIDTH + TOUCH_MIXER_CHANNEL_SPACING;
    int ch = channel_x / total_ch_width;
    
    if (ch >= 16) return 0;
    
    int ch_offset = channel_x % total_ch_width;
    if (ch_offset >= TOUCH_MIXER_CHANNEL_WIDTH) return 0;
    
    if (channel_id) *channel_id = ch;
    return 1;
}

// Handle touch input on mixer screen
void touch_handle_mixer_screen(AppState_Global *state, TouchEvent *event) {
    if (!state || !event) return;
    
    if (event->type == TOUCH_EVENT_NONE) return;
    
    int channel_id = 0;
    
    // Check what element was touched
    if (touch_is_on_fader(event->x, event->y, &channel_id)) {
        if (event->type == TOUCH_EVENT_PRESS || event->type == TOUCH_EVENT_MOVE) {
            // Calculate fader level from Y position
            int relative_y = event->y - TOUCH_MIXER_FADER_TOP;
            if (relative_y < 0) relative_y = 0;
            if (relative_y > TOUCH_MIXER_FADER_HEIGHT) relative_y = TOUCH_MIXER_FADER_HEIGHT;
            
            // Convert Y position to 0-100 level (inverted: top = 100, bottom = 0)
            int new_level = 100 - ((relative_y * 100) / TOUCH_MIXER_FADER_HEIGHT);
            state->channels[channel_id].fader_level = new_level;
            state->selected_channel = channel_id;
        }
        
    } else if (touch_is_on_mute_button(event->x, event->y, &channel_id)) {
        if (event->type == TOUCH_EVENT_RELEASE) {
            // Toggle mute on release (click behavior)
            state->channels[channel_id].mute = !state->channels[channel_id].mute;
            state->selected_channel = channel_id;
        }
        
    } else if (touch_is_on_eq_button(event->x, event->y, &channel_id)) {
        if (event->type == TOUCH_EVENT_RELEASE) {
            // Open EQ window on release
            state->eq_window.channel_id = channel_id;
            state->current_state = STATE_EQ_WINDOW;
        }
    }
}

// Main touch input dispatcher
void touch_input_dispatch(AppState_Global *state, TouchEvent *event) {
    if (!state || !event || event->type == TOUCH_EVENT_NONE) return;
    
    switch (state->current_state) {
        case STATE_MIXER_VIEW:
            touch_handle_mixer_screen(state, event);
            break;
        
        case STATE_MENU:
        case STATE_LOADING_SHOW:
        case STATE_CREATE_SHOW:
        case STATE_IP_CONFIG:
            // Menu touch handling would go here
            // For now, not implemented
            break;
        
        case STATE_EQ_WINDOW:
            // EQ window touch handling would go here
            break;
        
        default:
            break;
    }
}
