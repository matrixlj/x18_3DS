#ifndef NETWORK_CONFIG_WINDOW_H
#define NETWORK_CONFIG_WINDOW_H

#include "common.h"

// ============================================================================
// NETWORK CONFIG WINDOW STATE
// ============================================================================

extern int g_net_config_open;
extern char g_net_ip_digits[13];
extern char g_net_port_input[6];
extern int g_net_selected_field;
extern int g_net_digit_index;

// ============================================================================
// NETWORK CONFIG WINDOW FUNCTIONS
// ============================================================================

void render_net_config_window(void);
void handle_net_config_input(u32 kDown);
void load_network_config(void);
void save_network_config(void);
void ip_digits_to_display(const char *digits, char *display_buf, int max_len);

#endif
