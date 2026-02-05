#include "show_manager.h"

// Simple string copy
void simple_strcpy(char *dest, const char *src, int max_len) {
    int i = 0;
    while (i < max_len - 1 && src[i]) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// Simple string concatenation
void simple_strcat(char *dest, const char *src, int max_len) {
    int i = 0;
    while (dest[i] && i < max_len - 1) i++;
    
    int j = 0;
    while (i < max_len - 1 && src[j]) {
        dest[i++] = src[j++];
    }
    dest[i] = '\0';
}

// Simple string compare
int simple_strcmp(const char *s1, const char *s2) {
    int i = 0;
    while (s1[i] && s2[i] && s1[i] == s2[i]) {
        i++;
    }
    return s1[i] - s2[i];
}

ShowManager* show_manager_init() {
    // Allocate show manager - in real implementation would use malloc
    ShowManager *manager = (ShowManager*)0;  // Placeholder
    
    if (!manager) return 0;
    
    // Initialize default shows directory
    simple_strcpy(manager->shows_directory, "/3ds/app/shows", 256);
    
    manager->current_show = 0;
    manager->current_show_path[0] = '\0';
    
    return manager;
}

void show_manager_cleanup(ShowManager *manager) {
    if (!manager) return;
    
    if (manager->current_show) {
        db_free(manager->current_show);
        manager->current_show = 0;
    }
    
    // Free manager itself (would use free() in real implementation)
}

int show_manager_create_show(ShowManager *manager, const char *show_name, const char *mixer_ip) {
    if (!manager) return -1;
    
    // Free previous show if any
    if (manager->current_show) {
        db_free(manager->current_show);
    }
    
    // Create new database
    manager->current_show = db_create_new(show_name, mixer_ip);
    if (!manager->current_show) return -1;
    
    // Set path
    show_manager_get_show_path(manager, show_name, manager->current_show_path);
    
    return 0;
}

int show_manager_load_show(ShowManager *manager, const char *show_filename) {
    if (!manager) return -1;
    
    // Build full path
    char full_path[256];
    simple_strcpy(full_path, manager->shows_directory, 256);
    simple_strcat(full_path, "/", 256);
    simple_strcat(full_path, show_filename, 256);
    
    // Check if file exists (placeholder)
    if (!show_manager_file_exists(full_path)) {
        return -1;  // File not found
    }
    
    // Free previous show
    if (manager->current_show) {
        db_free(manager->current_show);
    }
    
    // Load from file
    manager->current_show = db_load_from_file(full_path);
    if (!manager->current_show) return -1;
    
    simple_strcpy(manager->current_show_path, full_path, 256);
    
    return 0;
}

int show_manager_save_show(ShowManager *manager) {
    if (!manager || !manager->current_show) return -1;
    
    if (manager->current_show_path[0] == '\0') {
        return -1;  // No path set
    }
    
    return db_save_to_file(manager->current_show, manager->current_show_path);
}

int show_manager_delete_show(ShowManager *manager, const char *show_filename) {
    // Placeholder - would:
    // 1. Build full path
    // 2. Check if it's the current show (if so, unload it)
    // 3. Delete the file
    
    if (!manager) return -1;
    
    // If deleting current show, unload it
    if (manager->current_show) {
        char filename_only[256];
        int i = 0;
        while (manager->current_show_path[i] && i < 256 - 1) {
            filename_only[i] = manager->current_show_path[i];
            i++;
        }
        filename_only[i] = '\0';
        
        if (simple_strcmp(show_filename, filename_only) == 0) {
            db_free(manager->current_show);
            manager->current_show = 0;
            manager->current_show_path[0] = '\0';
        }
    }
    
    return 0;  // Success placeholder
}

int show_manager_list_shows(ShowManager *manager, char **show_names, int max_shows) {
    // Placeholder - would:
    // 1. List files in shows_directory
    // 2. Filter for .db files
    // 3. Populate show_names array
    // 4. Return count
    
    if (!manager) return 0;
    
    return 0;  // Placeholder - 0 shows found
}

int show_manager_file_exists(const char *filepath) {
    // Placeholder - would check file existence via 3DS FS
    // For now, always return success
    return 1;
}

void show_manager_get_show_path(ShowManager *manager, const char *show_name, char *path) {
    if (!manager) return;
    
    simple_strcpy(path, manager->shows_directory, 256);
    simple_strcat(path, "/", 256);
    simple_strcat(path, show_name, 256);
    simple_strcat(path, ".db", 256);
}
