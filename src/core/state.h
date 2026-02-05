#ifndef STATE_H
#define STATE_H

#include "../core/constants.h"

// Application States
typedef enum {
    STATE_MIXER_VIEW,       // Main mixer view
    STATE_STEP_LIST_VIEW,   // Step list navigation (not used currently)
    STATE_EQ_WINDOW,        // EQ editing window
    STATE_MENU,             // Main menu (B button)
    STATE_LOADING_SHOW,     // Load show dialog
    STATE_CREATE_SHOW,      // Create show dialog
    STATE_IP_CONFIG         // Configure mixer IP
} AppState;

// Channel State for a single channel
typedef struct {
    int channel_id;         // 0-15
    char channel_name[32];
    int fader_level;        // 0-100 (was float 0.0-100.0)
    int mute;               // 0 or 1
    int eq_low;             // -20 to +20 dB (was float)
    int eq_mid;             // -20 to +20 dB (was float)
    int eq_high;            // -20 to +20 dB (was float)
} ChannelState;

// Step State
typedef struct {
    int step_id;
    int step_number;
    char step_name[64];
    ChannelState channels[NUM_CHANNELS];
} StepState;

// EQ Window State
typedef struct {
    int channel_id;
    float eq_low;
    float eq_mid;
    float eq_high;
    int selected_band;      // 0=Low, 1=Mid, 2=High
} EQWindowState;

// Menu State
typedef struct {
    int selected_option;    // 0=Load, 1=New, 2=IP Config, 3=Exit
} MenuState;

// Global Application State
typedef struct {
    AppState current_state;
    AppState previous_state;
    
    // Current data
    StepState current_step;
    ChannelState channels[NUM_CHANNELS];
    
    // Configuration
    char mixer_ip[16];      // IP address string
    int mixer_port;
    char current_show_path[256];
    
    // UI State
    int selected_channel;   // 0-15
    int selected_step;      // Step index
    
    // Sub-states
    EQWindowState eq_window;
    MenuState menu;
    
    // Flags
    int show_loaded;        // 0 or 1
    int modified;           // 1 if current step is modified
    int network_connected;  // 0 or 1
} AppState_Global;

// Global state instance
extern AppState_Global g_app_state;

// State initialization
void state_init();
void state_reset();
void state_set_state(AppState new_state);

#endif // STATE_H
