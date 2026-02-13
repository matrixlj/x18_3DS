#ifndef OPTIONS_WINDOW_H
#define OPTIONS_WINDOW_H

#include <stdint.h>

// Options structure
typedef struct {
    int send_fader;
    int send_eq;
} Options;

// Global options
extern Options g_options;
extern int g_options_window_open;
extern int g_options_selected_checkbox;

// Function declarations
void init_options(void);
void load_options(void);
void save_options(void);
void render_options_window(void);
void handle_options_input(u32 kDown);

#endif
