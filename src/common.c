#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

// ============================================================================
// GLOBAL STATE - SCREENS
// ============================================================================

C2D_Screen g_topScreen = {0};
C2D_Screen g_botScreen = {0};

// ============================================================================
// GLOBAL STATE - APP
// ============================================================================

int g_app_mode = 0;
int g_should_exit = 0;
int g_init_complete = 0;

// ============================================================================
// GLOBAL STATE - MIXER
// ============================================================================

int g_current_step = 0;
int g_selected_step = 0;
int g_selected_fader = 0;
struct sockaddr_in g_mixer_addr = {0};
char g_mixer_host[16] = "10.10.99.112";
int g_mixer_port = 10023;
int g_osc_connected = 0;
int g_osc_socket = -1;
int g_osc_verbose = 1;
Fader g_faders[NUM_FADERS] = {{0}};
int g_touched_fader_index = -1;
Show g_current_show = {{0}};
int g_show_loaded = 0;
int g_show_modified = 0;

// ============================================================================
// GLOBAL STATE - TOUCH INPUT
// ============================================================================

int g_isTouched = 0;
int g_wasTouched = 0;
touchPosition g_touchPos = {0};

// ============================================================================
// GLOBAL STATE - KEYBOARD/NEW SHOW
// ============================================================================

int g_creating_new_show = 0;
char g_new_show_name[64] = "";
int g_new_show_input_pos = 0;
char g_save_status[256] = "Ready";
int g_save_status_timer = 0;

// ============================================================================
// GLOBAL STATE - SHOW MANAGER
// ============================================================================

char g_available_shows[MAX_SHOWS][64] = {{0}};
int g_num_available_shows = 0;
int g_selected_show = 0;
char g_new_name[64] = "";
int g_renaming = 0;
int g_rename_input_pos = 0;

// ============================================================================
// GLOBAL STATE - EQ WINDOW
// ============================================================================

int g_eq_window_open = 0;
int g_eq_editing_channel = 0;
int g_eq_selected_band = 0;
int g_eq_param_selected = 0;

// ============================================================================
// GLOBAL STATE - NETWORK CONFIG
// ============================================================================

int g_net_config_open = 0;
char g_net_ip_digits[13] = "0";
char g_net_port_input[6] = "0";
int g_net_selected_field = 0;
int g_net_digit_index = 0;

// ============================================================================
// GRAPHICS RESOURCES
// ============================================================================

C2D_SpriteSheet g_grip_sheet = NULL;
C2D_Image g_grip_img = {0};
C2D_SpriteSheet g_fader_sheet = NULL;
C2D_Image g_fader_bkg = {0};
int g_romfs_mounted = 0;
int g_grip_loaded = 0;
int g_fader_loaded = 0;
C2D_TextBuf g_textBuf = NULL;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void draw_debug_text(C2D_Screen *screen, const char *text, float x, float y, float size, u32 color)
{
    if (!text || !g_textBuf) return;
    C2D_TextBufClear(g_textBuf);
    C2D_Text c2d_text;
    C2D_TextParse(&c2d_text, g_textBuf, text);
    C2D_TextOptimize(&c2d_text);
    C2D_DrawText(&c2d_text, C2D_WithColor, x, y, 0.5f, size, size, color);
}

void create_shows_directory(void)
{
    mkdir("/3ds", 0755);
    mkdir("/3ds/x18mixer", 0755);
}
