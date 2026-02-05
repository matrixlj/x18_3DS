#ifndef CONSTANTS_H
#define CONSTANTS_H

// 3DS Screen Dimensions
#define SCREEN_WIDTH_TOP 400
#define SCREEN_HEIGHT_TOP 240
#define SCREEN_WIDTH_BOTTOM 320
#define SCREEN_HEIGHT_BOTTOM 240

#define VRAM_BASE 0x18000000

// Mixer Configuration
#define NUM_CHANNELS 16
#define NUM_EQ_BANDS 3  // Low, Mid, High

// Colors (RGB565 format)
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_RED 0xF800
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE 0x001F
#define COLOR_GRAY 0x8410
#define COLOR_DARK_GRAY 0x4208
#define COLOR_CYAN 0x07FF
#define COLOR_YELLOW 0xFFE0

// UI Constants
#define CHANNEL_WIDTH 28
#define CHANNEL_SPACING 2
#define FADER_WIDTH 20
#define FADER_HEIGHT 100
#define BUTTON_SIZE 12

// OSC Configuration
#define DEFAULT_OSC_PORT 10023
#define OSC_BUFFER_SIZE 512

// Storage Paths
#define SHOWS_PATH "/3ds/app/shows"
#define CONFIG_FILE "/3ds/app/config.json"
#define DB_EXTENSION ".db"

#endif // CONSTANTS_H
