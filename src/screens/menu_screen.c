#include "menu_screen.h"
#include "../core/constants.h"

// Forward declarations
extern void fill_rect(int x1, int y1, int x2, int y2, unsigned short color, int screen);
extern void draw_pixel(int x, int y, unsigned short color, int screen);

void menu_draw_box(int x1, int y1, int x2, int y2, unsigned short color) {
    // Draw border - bottom screen
    // Top line
    for (int x = x1; x <= x2; x++) {
        draw_pixel(x, y1, color, 1);
    }
    // Bottom line
    for (int x = x1; x <= x2; x++) {
        draw_pixel(x, y2, color, 1);
    }
    // Left line
    for (int y = y1; y <= y2; y++) {
        draw_pixel(x1, y, color, 1);
    }
    // Right line
    for (int y = y1; y <= y2; y++) {
        draw_pixel(x2, y, color, 1);
    }
}

void menu_draw_text_simple(int x, int y, const char *text, unsigned short color) {
    // Placeholder for text rendering
    // Real implementation would use bitmap fonts
    // For now, just draw a small rectangle to indicate text position
    fill_rect(x, y, x + 50, y + 8, color, 1);
}

void menu_draw_button(int x, int y, int w, int h, const char *label, int selected, unsigned short color) {
    unsigned short button_color = selected ? COLOR_CYAN : COLOR_GRAY;
    
    // Draw button background
    fill_rect(x, y, x + w, y + h, button_color, 1);
    
    // Draw button border
    menu_draw_box(x, y, x + w, y + h, COLOR_WHITE);
    
    // Draw label (placeholder)
    menu_draw_text_simple(x + 5, y + 3, label, COLOR_BLACK);
}

void menu_render_main(AppState_Global *state) {
    // Clear bottom screen
    fill_rect(0, 0, SCREEN_WIDTH_BOTTOM - 1, SCREEN_HEIGHT_BOTTOM - 1, COLOR_BLACK, 1);
    
    // Draw main menu title area
    fill_rect(0, 0, SCREEN_WIDTH_BOTTOM - 1, 40, COLOR_DARK_GRAY, 1);
    menu_draw_text_simple(10, 10, "X18 Mixer Control", COLOR_WHITE);
    
    // Draw menu options
    int option_height = 40;
    int option_spacing = 50;
    int start_y = 60;
    
    // Load Show
    int selected = (state->menu.selected_option == 0) ? 1 : 0;
    menu_draw_button(30, start_y, 260, option_height, "Load Show", selected, COLOR_GRAY);
    
    // Create Show
    selected = (state->menu.selected_option == 1) ? 1 : 0;
    menu_draw_button(30, start_y + option_spacing, 260, option_height, "Create Show", selected, COLOR_GRAY);
    
    // IP Configuration
    selected = (state->menu.selected_option == 2) ? 1 : 0;
    menu_draw_button(30, start_y + 2 * option_spacing, 260, option_height, "Config IP", selected, COLOR_GRAY);
    
    // Exit
    selected = (state->menu.selected_option == 3) ? 1 : 0;
    menu_draw_button(30, start_y + 3 * option_spacing, 260, option_height, "Exit", selected, COLOR_RED);
}

void menu_render_load_show(AppState_Global *state) {
    // Clear bottom screen
    fill_rect(0, 0, SCREEN_WIDTH_BOTTOM - 1, SCREEN_HEIGHT_BOTTOM - 1, COLOR_BLACK, 1);
    
    // Draw dialog background
    fill_rect(20, 30, 300, 210, COLOR_DARK_GRAY, 1);
    menu_draw_box(20, 30, 300, 210, COLOR_WHITE);
    
    // Draw title
    menu_draw_text_simple(30, 40, "Load Show", COLOR_WHITE);
    
    // Draw placeholder list (would show actual shows)
    menu_draw_text_simple(30, 70, "Show 1", COLOR_WHITE);
    menu_draw_text_simple(30, 90, "Show 2", COLOR_WHITE);
    menu_draw_text_simple(30, 110, "Show 3", COLOR_WHITE);
    
    // Draw buttons
    menu_draw_button(30, 160, 100, 30, "OK", 0, COLOR_GREEN);
    menu_draw_button(150, 160, 100, 30, "Cancel", 0, COLOR_RED);
}

void menu_render_create_show(AppState_Global *state) {
    // Clear bottom screen
    fill_rect(0, 0, SCREEN_WIDTH_BOTTOM - 1, SCREEN_HEIGHT_BOTTOM - 1, COLOR_BLACK, 1);
    
    // Draw dialog background
    fill_rect(20, 30, 300, 210, COLOR_DARK_GRAY, 1);
    menu_draw_box(20, 30, 300, 210, COLOR_WHITE);
    
    // Draw title
    menu_draw_text_simple(30, 40, "Create Show", COLOR_WHITE);
    
    // Draw input fields (placeholders)
    menu_draw_text_simple(30, 70, "Show Name:", COLOR_WHITE);
    fill_rect(30, 85, 280, 100, COLOR_DARK_GRAY, 1);
    menu_draw_box(30, 85, 280, 100, COLOR_CYAN);
    
    menu_draw_text_simple(30, 110, "Mixer IP:", COLOR_WHITE);
    fill_rect(30, 125, 280, 140, COLOR_DARK_GRAY, 1);
    menu_draw_box(30, 125, 280, 140, COLOR_CYAN);
    
    // Draw buttons
    menu_draw_button(30, 160, 100, 30, "Create", 0, COLOR_GREEN);
    menu_draw_button(150, 160, 100, 30, "Cancel", 0, COLOR_RED);
}

void menu_render_ip_config(AppState_Global *state) {
    // Clear bottom screen
    fill_rect(0, 0, SCREEN_WIDTH_BOTTOM - 1, SCREEN_HEIGHT_BOTTOM - 1, COLOR_BLACK, 1);
    
    // Draw dialog background
    fill_rect(20, 30, 300, 210, COLOR_DARK_GRAY, 1);
    menu_draw_box(20, 30, 300, 210, COLOR_WHITE);
    
    // Draw title
    menu_draw_text_simple(30, 40, "Configure IP", COLOR_WHITE);
    
    // Draw current IP
    menu_draw_text_simple(30, 70, "Mixer IP:", COLOR_WHITE);
    fill_rect(30, 85, 280, 100, COLOR_DARK_GRAY, 1);
    menu_draw_box(30, 85, 280, 100, COLOR_CYAN);
    
    // Draw port
    menu_draw_text_simple(30, 110, "Port: 10023", COLOR_WHITE);
    
    // Draw buttons
    menu_draw_button(30, 160, 100, 30, "Save", 0, COLOR_GREEN);
    menu_draw_button(150, 160, 100, 30, "Cancel", 0, COLOR_RED);
}

void menu_handle_input(AppState_Global *state, int button) {
    // Handle input based on current menu state
    // Button codes would be:
    // 0 = A button
    // 1 = B button
    // 2 = X button
    // 3 = Y button
    // 4 = D-Pad Up
    // 5 = D-Pad Down
    
    if (button == 1) {  // B button - go back
        state_set_state(STATE_MIXER_VIEW);
        return;
    }
    
    if (button == 4) {  // D-Pad Up
        if (state->menu.selected_option > 0) {
            state->menu.selected_option--;
        }
    } else if (button == 5) {  // D-Pad Down
        if (state->menu.selected_option < 3) {
            state->menu.selected_option++;
        }
    } else if (button == 0) {  // A button - select option
        switch (state->menu.selected_option) {
            case 0:  // Load Show
                state_set_state(STATE_LOADING_SHOW);
                break;
            case 1:  // Create Show
                state_set_state(STATE_CREATE_SHOW);
                break;
            case 2:  // IP Config
                state_set_state(STATE_IP_CONFIG);
                break;
            case 3:  // Exit
                // Signal exit
                break;
        }
    }
}
