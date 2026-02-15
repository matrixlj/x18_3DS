#ifndef SHOW_MANAGER_WINDOW_H
#define SHOW_MANAGER_WINDOW_H

#include "common.h"

// ============================================================================
// SHOW MANAGER STATE
// ============================================================================

extern int g_selected_show;
extern char g_new_name[64];
extern int g_renaming;
extern int g_rename_input_pos;
extern int g_show_modified;

// ============================================================================
// SHOW MANAGER WINDOW FUNCTIONS
// ============================================================================

void render_show_manager(void);
void handle_manager_input(void);
int check_button_touch(int button_idx);
int get_show_item_from_touch(void);

// Rename window functions
void render_rename_window_top(void);
void render_rename_window_bot(void);
void handle_rename_input(void);

#endif
