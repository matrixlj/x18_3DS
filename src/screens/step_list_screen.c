#include "step_list_screen.h"
#include "../core/constants.h"

// Forward declarations from main.c
void fill_rect(int x1, int y1, int x2, int y2, unsigned short color, int screen);
void draw_pixel(int x, int y, unsigned short color, int screen);

// Calculate which step should be displayed at top of visible list
int step_list_get_display_offset(int selected_step, int visible_items) {
    // Ensure selected step is visible in the list
    // If selected_step is in top half of visible items, show from 0
    // If selected_step is in bottom half, scroll to show it in middle
    
    int offset = selected_step - (visible_items / 2);
    if (offset < 0) offset = 0;
    
    // Max steps in a show is 100, so don't scroll beyond that
    int max_offset = 100 - visible_items;
    if (max_offset < 0) max_offset = 0;
    if (offset > max_offset) offset = max_offset;
    
    return offset;
}

// Main step list screen rendering (top screen)
void step_list_render_screen(AppState_Global *state) {
    // Clear top screen
    fill_rect(0, 0, SCREEN_WIDTH_TOP - 1, SCREEN_HEIGHT_TOP - 1, COLOR_BLACK, 0);
    
    // Draw title bar
    fill_rect(0, 0, SCREEN_WIDTH_TOP - 1, 15, COLOR_DARK_GRAY, 0);
    
    // Title "STEPS" would be drawn here (text rendering placeholder)
    // For now, just a visual indicator
    draw_pixel(10, 5, COLOR_WHITE, 0);
    
    // Draw info bar (current step info)
    step_list_render_info_bar(state);
    
    // Calculate display offset for scrolling
    int offset = step_list_get_display_offset(state->selected_step, STEP_LIST_VISIBLE_ITEMS);
    
    // Draw step list items
    int y_pos = STEP_LIST_TOP_MARGIN + 20;  // Below title bar
    
    for (int i = 0; i < STEP_LIST_VISIBLE_ITEMS && (offset + i) < 100; i++) {
        int step_num = offset + i;
        int is_selected = (step_num == state->selected_step) ? 1 : 0;
        
        // Generate simple step name
        char step_name[32];
        step_name[0] = 'S';
        step_name[1] = 't';
        step_name[2] = 'e';
        step_name[3] = 'p';
        step_name[4] = ' ';
        
        int name_idx = 5;
        // Add step number (1-indexed)
        int display_num = step_num + 1;
        if (display_num >= 100) {
            step_name[name_idx++] = '0' + (display_num / 100);
            step_name[name_idx++] = '0' + ((display_num / 10) % 10);
        } else if (display_num >= 10) {
            step_name[name_idx++] = '0' + (display_num / 10);
        }
        step_name[name_idx++] = '0' + (display_num % 10);
        step_name[name_idx] = '\0';
        
        step_list_render_item(STEP_LIST_LEFT_MARGIN, y_pos, step_num, step_name, is_selected);
        
        y_pos += STEP_LIST_ITEM_HEIGHT;
    }
}

// Render info bar showing current step details
void step_list_render_info_bar(AppState_Global *state) {
    // Draw info bar background (below title)
    fill_rect(0, 16, SCREEN_WIDTH_TOP - 1, 35, COLOR_DARK_GRAY, 0);
    
    // Info would display: "Current: Step X | Channels: 16"
    // Placeholder text rendering
    
    // Draw divider line
    fill_rect(0, 35, SCREEN_WIDTH_TOP - 1, 36, COLOR_GRAY, 0);
}

// Render single step list item
void step_list_render_item(int x, int y, int step_num, const char *step_name, int selected) {
    unsigned short bg_color = selected ? COLOR_DARK_GRAY : COLOR_BLACK;
    unsigned short border_color = selected ? COLOR_CYAN : COLOR_GRAY;
    unsigned short text_color = selected ? COLOR_CYAN : COLOR_WHITE;
    
    // Draw item background
    fill_rect(x, y, SCREEN_WIDTH_TOP - STEP_LIST_RIGHT_MARGIN, y + STEP_LIST_ITEM_HEIGHT - 2, bg_color, 0);
    
    // Draw selection border if selected
    if (selected) {
        // Top border
        fill_rect(x, y, SCREEN_WIDTH_TOP - STEP_LIST_RIGHT_MARGIN, y + 1, border_color, 0);
        // Left border
        fill_rect(x, y, x + 2, y + STEP_LIST_ITEM_HEIGHT - 2, border_color, 0);
        // Right border
        fill_rect(SCREEN_WIDTH_TOP - STEP_LIST_RIGHT_MARGIN - 2, y, SCREEN_WIDTH_TOP - STEP_LIST_RIGHT_MARGIN, y + STEP_LIST_ITEM_HEIGHT - 2, border_color, 0);
    }
    
    // Draw step number (simple pixel representation)
    // In real version, would use proper font rendering
    for (int i = 0; i < 4; i++) {
        draw_pixel(x + 5 + i, y + 10, text_color, 0);
    }
    
    // Draw step name indicator
    for (int i = 0; i < 15; i++) {
        if ((i % 2) == 0) {
            draw_pixel(x + 30 + i, y + 10, text_color, 0);
        }
    }
    
    // Draw divider line
    fill_rect(x, y + STEP_LIST_ITEM_HEIGHT - 1, SCREEN_WIDTH_TOP - STEP_LIST_RIGHT_MARGIN, y + STEP_LIST_ITEM_HEIGHT, COLOR_GRAY, 0);
}

// Handle button input for step list
void step_list_handle_button_input(AppState_Global *state, int button) {
    switch (button) {
        case 0: // D-Pad Left
            // Go to previous step
            if (state->selected_step > 0) {
                state->selected_step--;
            }
            break;
        
        case 1: // D-Pad Right
            // Go to next step
            if (state->selected_step < 99) {
                state->selected_step++;
            }
            break;
        
        case 2: // D-Pad Up
            // Go to previous step (same as left)
            if (state->selected_step > 0) {
                state->selected_step--;
            }
            break;
        
        case 3: // D-Pad Down
            // Go to next step (same as right)
            if (state->selected_step < 99) {
                state->selected_step++;
            }
            break;
        
        case 4: // A button - Confirm step selection
            // Already on the step, just acknowledge
            break;
        
        case 5: // B button - Close step list (would go back to mixer)
            // In this case, just stay in current state
            break;
        
        case 6: // X button
            // Delete current step (advanced feature)
            break;
        
        case 7: // Y button
            // Create new step after current
            if (state->selected_step < 99) {
                state->selected_step++;
            }
            break;
    }
}
