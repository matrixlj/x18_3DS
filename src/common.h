#ifndef COMMON_H
#define COMMON_H

#include <citro2d.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "types.h"

// ============================================================================
// SCREEN DEFINITIONS
// ============================================================================

#define SCREEN_WIDTH_TOP    400
#define SCREEN_HEIGHT_TOP   240
#define SCREEN_WIDTH_BOT    320
#define SCREEN_HEIGHT_BOT   240

// ============================================================================
// APP MODES
// ============================================================================

#define APP_MODE_MIXER   0
#define APP_MODE_MANAGER 1

// ============================================================================
// STRUCTURES
// ============================================================================

typedef struct {
    C3D_RenderTarget *target;
    int width;
    int height;
} C2D_Screen;

// ============================================================================
// GLOBAL STATE - SCREENS
// ============================================================================

extern C2D_Screen g_topScreen;
extern C2D_Screen g_botScreen;

// ============================================================================
// GLOBAL STATE - APP
// ============================================================================

extern int g_app_mode;
extern int g_should_exit;
extern int g_init_complete;

// ============================================================================
// GLOBAL STATE - MIXER
// ============================================================================

extern int g_current_step;
extern int g_selected_step;
extern int g_selected_fader;
extern struct sockaddr_in g_mixer_addr;
extern char g_mixer_host[16];
extern int g_mixer_port;
extern int g_osc_connected;
extern int g_osc_socket;
extern int g_osc_verbose;
extern Fader g_faders[NUM_FADERS];
extern int g_touched_fader_index;
extern Show g_current_show;
extern int g_show_loaded;
extern int g_show_modified;

// ============================================================================
// GLOBAL STATE - KEYBOARD/NEW SHOW
// ============================================================================

extern int g_creating_new_show;
extern char g_new_show_name[64];
extern int g_new_show_input_pos;
extern char g_save_status[256];
extern int g_save_status_timer;

// ============================================================================
// GLOBAL STATE - SHOW MANAGER
// ============================================================================

extern char g_available_shows[MAX_SHOWS][64];
extern int g_num_available_shows;
extern int g_selected_show;
extern char g_new_name[64];
extern int g_renaming;
extern int g_rename_input_pos;

// ============================================================================
// GLOBAL STATE - EQ WINDOW
// ============================================================================

extern int g_eq_window_open;
extern int g_eq_editing_channel;
extern int g_eq_selected_band;
extern int g_eq_param_selected;

// ============================================================================
// GLOBAL STATE - NETWORK CONFIG
// ============================================================================

extern int g_net_config_open;
extern char g_net_ip_digits[13];
extern char g_net_port_input[6];
extern int g_net_selected_field;
extern int g_net_digit_index;

// ============================================================================
// GLOBAL STATE - TOUCH INPUT
// ============================================================================

extern int g_isTouched;
extern int g_wasTouched;
extern touchPosition g_touchPos;

// ============================================================================
// GRAPHICS RESOURCES
// ============================================================================

extern C2D_SpriteSheet g_grip_sheet;
extern C2D_Image g_grip_img;
extern C2D_SpriteSheet g_fader_sheet;
extern C2D_Image g_fader_bkg;
extern int g_romfs_mounted;
extern int g_grip_loaded;
extern int g_fader_loaded;
extern C2D_TextBuf g_textBuf;

// ============================================================================
// UTILITY FUNCTIONS (in common.c)
// ============================================================================

void draw_debug_text(C2D_Screen *screen, const char *text, float x, float y, float size, u32 color);
void create_shows_directory(void);

#endif
