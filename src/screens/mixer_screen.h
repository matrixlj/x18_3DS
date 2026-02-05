#ifndef MIXER_SCREEN_H
#define MIXER_SCREEN_H

#include "../core/state.h"

// Mixer channel widget dimensions
#define MIXER_CHANNEL_WIDTH 20
#define MIXER_CHANNEL_SPACING 2
#define MIXER_FADER_HEIGHT 80
#define MIXER_BUTTON_SIZE 10
#define MIXER_TOP_MARGIN 10
#define MIXER_BOTTOM_MARGIN 20

// Layout calculation
#define MIXER_TOTAL_WIDTH (NUM_CHANNELS * (MIXER_CHANNEL_WIDTH + MIXER_CHANNEL_SPACING))
#define MIXER_FADER_TOP 30
#define MIXER_MUTE_TOP (MIXER_FADER_TOP + MIXER_FADER_HEIGHT + 5)
#define MIXER_EQ_TOP (MIXER_MUTE_TOP + MIXER_BUTTON_SIZE + 3)

// Mixer rendering functions
void mixer_render_screen(AppState_Global *state);
void mixer_render_channel(int channel_id, ChannelState *state, int x, int selected);
void mixer_render_fader(int x, int y, int level, unsigned short color);
void mixer_render_button(int x, int y, int size, const char *label, int pressed, unsigned short color);

// Mixer input handling
void mixer_handle_button_input(AppState_Global *state, int button);
void mixer_handle_touch_input(AppState_Global *state, int x, int y);
void mixer_adjust_fader(ChannelState *channel, int delta);

// Utility
int mixer_get_channel_x(int channel_id);
int mixer_is_touch_on_channel(int x, int y, int *channel_id, int *element);

#endif // MIXER_SCREEN_H
