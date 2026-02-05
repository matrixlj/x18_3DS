#include "state.h"

// Global application state
AppState_Global g_app_state;

// Simple memset replacement
void* simple_memset(void* ptr, int value, int size) {
    unsigned char* p = (unsigned char*)ptr;
    for (int i = 0; i < size; i++) {
        p[i] = (unsigned char)value;
    }
    return ptr;
}

// Simple strcpy replacement
char* simple_strncpy(char* dest, const char* src, int size) {
    int i = 0;
    while (i < size - 1 && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

// Simple snprintf replacement (simplified - no modulo division)
int simple_snprintf(char* buf, int size, const char* fmt, int arg) {
    int offset = 0;
    
    while (*fmt && offset < size - 1) {
        if (*fmt == '%' && *(fmt + 1) == 'd') {
            // Simple integer formatting - just convert to string
            int num = arg;
            if (num >= 0 && num <= 99) {
                if (num >= 10) {
                    buf[offset++] = '0' + (num / 10);
                    if (offset < size - 1) {
                        buf[offset++] = '0' + (num - (num / 10) * 10);
                    }
                } else {
                    buf[offset++] = '0' + num;
                }
            }
            fmt += 2;
        } else {
            buf[offset++] = *fmt;
            fmt++;
        }
    }
    
    buf[offset] = '\0';
    return offset;
}

void state_init() {
    simple_memset(&g_app_state, 0, sizeof(AppState_Global));
    
    // Initialize default values
    g_app_state.current_state = STATE_MENU;
    g_app_state.previous_state = STATE_MENU;
    g_app_state.mixer_port = DEFAULT_OSC_PORT;
    g_app_state.selected_channel = 0;
    g_app_state.selected_step = 0;
    g_app_state.show_loaded = 0;
    g_app_state.modified = 0;
    g_app_state.network_connected = 0;
    
    // Initialize channels with default values
    for (int i = 0; i < NUM_CHANNELS; i++) {
        g_app_state.channels[i].channel_id = i;
        simple_snprintf(g_app_state.channels[i].channel_name, 32, "Channel %d", i + 1);
        g_app_state.channels[i].fader_level = 0.0f;
        g_app_state.channels[i].mute = 0;
        g_app_state.channels[i].eq_low = 0.0f;
        g_app_state.channels[i].eq_mid = 0.0f;
        g_app_state.channels[i].eq_high = 0.0f;
    }
    
    // Initialize default mixer IP
    simple_strncpy(g_app_state.mixer_ip, "192.168.1.100", 15);
}

void state_reset() {
    // Reset to initial state
    state_init();
}

void state_set_state(AppState new_state) {
    if (new_state != g_app_state.current_state) {
        g_app_state.previous_state = g_app_state.current_state;
        g_app_state.current_state = new_state;
    }
}
