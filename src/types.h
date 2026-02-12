#ifndef TYPES_H
#define TYPES_H

#include <citro2d.h>

// ============================================================================
// MIXER DEFINITIONS
// ============================================================================

#define NUM_FADERS 16
#define FADERS_PER_ROW 16
#define FADER_WIDTH (SCREEN_WIDTH_BOT / FADERS_PER_ROW)
#define FADER_HEIGHT SCREEN_HEIGHT_BOT
#define FADER_BAR_WIDTH 3
#define BUTTON_HEIGHT 14

// ============================================================================
// SHOW MANAGER DEFINITIONS
// ============================================================================

#define SHOWS_DIR "/sdmc/3ds/x18mixer/shows/"
#define MAX_SHOWS 64

// Manager buttons
#define BTN_LOAD 0
#define BTN_DELETE 1
#define BTN_DUPLICATE 2
#define BTN_RENAME 3
#define BTN_NET 4
#define NUM_BUTTONS 5

// ============================================================================
// FADER STRUCTURE
// ============================================================================

typedef struct {
    int id;
    float value;
    int muted;
    int eq_enabled;
    float x, y;
    float w, h;
} Fader;

// ============================================================================
// EQ DEFINITIONS
// ============================================================================

// EQ Filter Types (matching X18 OSC spec: LCut, LShv, PEQ, VEQ, HShv, HCut)
typedef enum {
    EQ_LCUT,   // Low Cut (High-Pass) - /ch/01/eq/1/type = 0
    EQ_LSHV,   // Low Shelf - /ch/01/eq/1/type = 1
    EQ_PEQ,    // Peaking EQ - /ch/01/eq/1/type = 2
    EQ_VPEQ,   // Vintage/Variable PEQ - /ch/01/eq/1/type = 3
    EQ_HSHV,   // High Shelf - /ch/01/eq/1/type = 4
    EQ_HCUT    // High Cut (Low-Pass) - /ch/01/eq/1/type = 5
} EQFilterType;

// Single EQ Band
typedef struct {
    float frequency;      // Hz (20-20000)
    float gain;           // dB (-15 to +15)
    float q_factor;       // 0.3 to 10.0
    EQFilterType type;    // Filter type
} __attribute__((packed)) EQBand;

// EQ Settings for a single channel
typedef struct {
    EQBand bands[5];      // 5 parametric bands
    int enabled;          // 0=disabled, 1=enabled
} __attribute__((packed)) ChannelEQ;

// ============================================================================
// SHOW/STEP STRUCTURES
// ============================================================================

typedef struct {
    char name[32];
    float volumes[16];
    int mutes[16];
    ChannelEQ eqs[16];    // EQ settings per channel
} __attribute__((packed)) Step;

typedef struct {
    char name[64];
    Step steps[200];
    int num_steps;
    int magic;  // Magic number for validation: 0x58334D32 ('X', '3', '4', 'M') = X34M = X18Mix ver 2
} __attribute__((packed)) Show;

#endif
