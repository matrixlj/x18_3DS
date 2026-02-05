#include "core/state.h"
#include "core/constants.h"

// Screen buffer
static unsigned short *fb_top = (unsigned short *)VRAM_BASE;
static unsigned short *fb_bottom = (unsigned short *)(VRAM_BASE + 0x46500);

// Hardware registers
#define LCD_POWERCNT        ((volatile unsigned int*)(0x10202000 + 0x10))
#define LCD_COLORFILL       ((volatile unsigned int*)(0x10202000 + 0x14))

// Function prototypes
void render_frame();
void handle_input();
void update_logic();
void cleanup();
void draw_pixel(int x, int y, unsigned short color, int screen);
void fill_rect(int x1, int y1, int x2, int y2, unsigned short color, int screen);
void wait_vblank();

// Helper: draw pixel
void draw_pixel(int x, int y, unsigned short color, int screen) {
    unsigned short *fb = (screen == 0) ? fb_top : fb_bottom;
    int width = (screen == 0) ? SCREEN_WIDTH_TOP : SCREEN_WIDTH_BOTTOM;
    int height = SCREEN_HEIGHT_TOP;
    
    if (x >= 0 && x < width && y >= 0 && y < height) {
        fb[y * width + x] = color;
    }
}

// Helper: fill rectangle
void fill_rect(int x1, int y1, int x2, int y2, unsigned short color, int screen) {
    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            draw_pixel(x, y, color, screen);
        }
    }
}

// Simple memset for framebuffer
void simple_memset_fb(unsigned short *ptr, unsigned short value, int count) {
    for (int i = 0; i < count; i++) {
        ptr[i] = value;
    }
}

// Wait for vblank
void wait_vblank(void) {
    static volatile unsigned int* gpu_stat = (volatile unsigned int*)0x10400010;
    while (*gpu_stat & 0x01);
    while (!(*gpu_stat & 0x01));
}

// Input handling
void handle_button_a() {
    if (g_app_state.current_state == STATE_MIXER_VIEW) {
        g_app_state.modified = 0;
    }
}

void handle_button_b() {
    state_set_state(STATE_MENU);
}

void handle_button_x() {
    if (g_app_state.current_state == STATE_MIXER_VIEW) {
        g_app_state.modified = 0;
    }
}

void handle_button_y() {
    if (g_app_state.current_state == STATE_MIXER_VIEW) {
        // Create new step
    }
}

void handle_dpad_up() {
    if (g_app_state.selected_step > 0) {
        g_app_state.selected_step--;
    }
}

void handle_dpad_down() {
    g_app_state.selected_step++;
}

void handle_input() {
    // Placeholder - would read from 3DS HID module
}

void render_frame() {
    // Clear screens
    simple_memset_fb(fb_top, COLOR_BLACK, SCREEN_WIDTH_TOP * SCREEN_HEIGHT_TOP);
    simple_memset_fb(fb_bottom, COLOR_BLACK, SCREEN_WIDTH_BOTTOM * SCREEN_HEIGHT_BOTTOM);
    
    // Render based on current state
    switch (g_app_state.current_state) {
        case STATE_MIXER_VIEW:
            // Draw mixer interface
            fill_rect(10, 20, 310, 220, COLOR_DARK_GRAY, 1);
            break;
        case STATE_MENU:
            // Draw menu
            fill_rect(50, 50, 270, 190, COLOR_GRAY, 1);
            break;
        case STATE_EQ_WINDOW:
            // Draw EQ window
            fill_rect(30, 30, 290, 210, COLOR_GRAY, 1);
            break;
        default:
            break;
    }
}

void update_logic() {
    // Update game logic
    switch (g_app_state.current_state) {
        case STATE_MIXER_VIEW:
            if (g_app_state.modified) {
                // Step modified
            }
            break;
        default:
            break;
    }
}

int main(void) {
    // Initialize application state
    state_init();
    
    // Enable displays
    *LCD_POWERCNT |= 0x01000001;
    
    // Set initial state
    state_set_state(STATE_MIXER_VIEW);
    
    // Main loop
    int frame_count = 0;
    int running = 1;
    
    while (running) {
        frame_count++;
        
        // Handle input
        handle_input();
        
        // Update logic
        update_logic();
        
        // Render frame
        render_frame();
        
        // Wait for vblank
        wait_vblank();
        
        // Run for limited frames in test mode
        if (frame_count >= 300) {
            running = 0;
        }
    }
    
    cleanup();
    
    return 0;
}

void cleanup() {
    // Cleanup code
}
