#ifndef TOUCH_INPUT_H
#define TOUCH_INPUT_H

#include "../core/state.h"

// Touch screen hardware configuration
#define TOUCH_SCREEN_WIDTH 320
#define TOUCH_SCREEN_HEIGHT 240

// Touch event types
typedef enum {
    TOUCH_EVENT_NONE = 0,
    TOUCH_EVENT_PRESS = 1,
    TOUCH_EVENT_MOVE = 2,
    TOUCH_EVENT_RELEASE = 3
} TouchEventType;

// Touch event structure
typedef struct {
    TouchEventType type;
    int x;
    int y;
    int duration;  // Frames pressed
} TouchEvent;

// Touch input manager
typedef struct {
    int is_pressed;         // Currently pressed or not
    int last_x;             // Last known X coordinate
    int last_y;             // Last known Y coordinate
    int press_frame;        // Frame when pressed started
    int current_frame;      // Current frame count
    TouchEvent last_event;  // Last event processed
} TouchInputManager;

// Touch input functions
void touch_input_init(TouchInputManager *manager);
void touch_input_update(TouchInputManager *manager, int new_x, int new_y, int is_pressed);
TouchEvent touch_input_get_event(TouchInputManager *manager);

// Touch coordinate validation
int touch_is_valid_coords(int x, int y);

// Touch event helper functions
int touch_is_on_fader(int x, int y, int *channel_id);
int touch_is_on_mute_button(int x, int y, int *channel_id);
int touch_is_on_eq_button(int x, int y, int *channel_id);

// Touch handling for mixer screen
void touch_handle_mixer_screen(AppState_Global *state, TouchEvent *event);

// Touch handling dispatcher
void touch_input_dispatch(AppState_Global *state, TouchEvent *event);

#endif // TOUCH_INPUT_H
