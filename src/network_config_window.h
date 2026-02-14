#ifndef NETWORK_CONFIG_WINDOW_H
#define NETWORK_CONFIG_WINDOW_H

#include "common.h"

// ============================================================================
// NETWORK CONFIG WINDOW STATE
// ============================================================================

extern int g_net_config_open;
extern char g_net_ip_input[64];
extern char g_net_port_input[6];
extern int g_net_selected_field;
extern int g_net_keyboard_selected;
extern u32 g_net_last_key_time;

// ============================================================================
// NETWORK CONFIG WINDOW FUNCTIONS
// ============================================================================

void render_net_config_window(void);
void handle_net_config_input(u32 kDown);
void handle_net_config_touch(touchPosition touch);
void load_network_config(void);
void save_network_config(void);
void load_network_config_from_file(void);
int is_valid_ip(const char *ip);

#endif
