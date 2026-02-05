#ifndef SHOW_MANAGER_H
#define SHOW_MANAGER_H

#include "database.h"

// Show manager - high-level API for managing shows
typedef struct {
    char shows_directory[256];
    Database *current_show;
    char current_show_path[256];
} ShowManager;

// Initialization
ShowManager* show_manager_init();
void show_manager_cleanup(ShowManager *manager);

// Show operations
int show_manager_create_show(ShowManager *manager, const char *show_name, const char *mixer_ip);
int show_manager_load_show(ShowManager *manager, const char *show_filename);
int show_manager_save_show(ShowManager *manager);
int show_manager_delete_show(ShowManager *manager, const char *show_filename);

// List shows available
int show_manager_list_shows(ShowManager *manager, char **show_names, int max_shows);

// Utility
int show_manager_file_exists(const char *filepath);
void show_manager_get_show_path(ShowManager *manager, const char *show_name, char *path);

#endif // SHOW_MANAGER_H
