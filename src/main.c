#include <citro2d.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <3ds.h>

// Module headers
#include "types.h"
#include "common.h"
#include "renderer.h"
#include "mixer_window.h"
#include "keyboard_window.h"
#include "show_manager_window.h"
#include "network_config_window.h"
#include "eq_window.h"
#include "options_window.h"

// ============================================================================
// SOCKET BUFFER (for socInit on 3DS)
// ============================================================================

// 1MB buffer for socket operations - use smaller buffer aligned properly
static u32 SOC_BUFFER[0x80000 / 4] __attribute__((aligned(0x1000)));

// ============================================================================
// SCREEN DEFINITIONS
// ============================================================================

#define SCREEN_WIDTH_TOP    400
#define SCREEN_HEIGHT_TOP   240
#define SCREEN_WIDTH_BOT    320
#define SCREEN_HEIGHT_BOT   240

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

// SHOWS_DIR is defined in types.h (avoid duplication)
#define MAX_SHOWS 64
#define APP_MODE_MIXER 0
#define APP_MODE_MANAGER 1

// Manager buttons
// Manager buttons
#define BTN_LOAD 0
#define BTN_DELETE 1
#define BTN_DUPLICATE 2
#define BTN_RENAME 3
#define BTN_NET 4
#define NUM_BUTTONS 5

// All type definitions moved to types.h
// All Screen definitions moved to common.h
// All global state instantiated in common.c

// Note: The global variable definitions have been moved to common.c
// to ensure single instantiation across all modules.
// Include common.h to get extern declarations for all globals.

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

int load_show_from_file(const char *filename, Show *out_show);
void apply_step_to_faders(int step_idx);
void save_show_to_file(Show *show);
void save_channel_eq_only(int channel);
void add_step(void);
void duplicate_step(void);
void load_network_config(void);
void save_network_config(void);
void handle_net_config_input(u32 kDown);
void render_net_config_window(void);
void ip_digits_to_display(const char *digits, char *display_buf, int max_len);

// ============================================================================
// OSC CORE FUNCTIONS (Phase 1 - Send Only)
// ============================================================================

// Initialize OSC connection
void osc_init(void)
{
    FILE *dbg = fopen("/3ds/x18mixer/osc_debug.txt", "w");
    if (dbg) fprintf(dbg, "[OSC_INIT] Starting OSC initialization...\n");
    
    // Create UDP socket
    g_osc_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (dbg) fprintf(dbg, "[OSC_INIT] socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) returned: %d\n", g_osc_socket);
    
    if (g_osc_socket < 0) {
        if (dbg) fprintf(dbg, "[OSC_INIT] ERROR: Failed to create socket\n");
        if (dbg) fclose(dbg);
        return;
    }
    
    if (g_osc_socket < 0) {
        if (dbg) fprintf(dbg, "[OSC_INIT] ERROR: Failed to create socket\n");
        if (dbg) fclose(dbg);
        return;
    }
    
    // Setup mixer address
    memset(&g_mixer_addr, 0, sizeof(g_mixer_addr));
    g_mixer_addr.sin_family = AF_INET;
    g_mixer_addr.sin_port = htons(g_mixer_port);
    
    if (dbg) fprintf(dbg, "[OSC_INIT] Parsing IP: '%s' (len=%d), Port: %d\n", g_mixer_host, (int)strlen(g_mixer_host), g_mixer_port);
    
    int ret = inet_pton(AF_INET, g_mixer_host, &g_mixer_addr.sin_addr);
    if (dbg) fprintf(dbg, "[OSC_INIT] inet_pton returned: %d\n", ret);
    
    if (ret <= 0) {
        if (dbg) fprintf(dbg, "[OSC_INIT] ERROR: Invalid IP address: '%s' (ret=%d)\n", g_mixer_host, ret);
        if (dbg) fclose(dbg);
        close(g_osc_socket);
        g_osc_socket = -1;
        return;
    }
    
    g_osc_connected = 1;
    if (dbg) fprintf(dbg, "[OSC_INIT] SUCCESS! Connected to %s:%d, socket=%d\n", g_mixer_host, g_mixer_port, g_osc_socket);
    if (dbg) fclose(dbg);
}

// Send OSC message (generic)
int osc_send(const uint8_t *packet, int packet_size)
{
    if (g_osc_socket < 0 || !g_osc_connected) {
        return -1;
    }
    
    int n = sendto(g_osc_socket, packet, packet_size, 0, 
                   (struct sockaddr*)&g_mixer_addr, sizeof(g_mixer_addr));
    
    return n;
}

// Send fader value for a channel
void osc_send_fader(int channel, float value)
{
    // Validate channel
    if (channel < 0 || channel >= 16) return;
    
    // Build OSC message: /ch/XX/mix/fader ,f <value>
    // Format: address(null-padded) + ",f\0\0" + float(4 bytes)
    
    uint8_t packet[32];
    int pos = 0;
    
    // Address: /ch/XX/mix/fader
    pos += snprintf((char*)&packet[pos], 30, "/ch/%02d/mix/fader", channel + 1);
    pos++;  // null terminator
    
    // Pad to 4-byte boundary
    while (pos % 4 != 0) {
        packet[pos++] = 0;
    }
    
    // Type tag: ,f
    packet[pos++] = ',';
    packet[pos++] = 'f';
    packet[pos++] = 0;
    packet[pos++] = 0;
    
    // Float value (big-endian)
    uint32_t float_bits = *(uint32_t*)&value;
    packet[pos++] = (float_bits >> 24) & 0xFF;
    packet[pos++] = (float_bits >> 16) & 0xFF;
    packet[pos++] = (float_bits >> 8) & 0xFF;
    packet[pos++] = float_bits & 0xFF;
    
    int result = osc_send(packet, pos);
    printf("[DEBUG] OSC_SEND_FADER CH%02d: value=%.2f, result=%d, connected=%d, socket=%d\n", 
           channel + 1, value, result, g_osc_connected, g_osc_socket);
}

// Send mute state for a channel
void osc_send_mute(int channel, int muted)
{
    // Validate channel
    if (channel < 0 || channel >= 16) return;
    
    // Build OSC message: /ch/XX/mix/on ,i <value>
    // Syntax: 0=muted, 1=unmuted
    
    uint8_t packet[32];
    int pos = 0;
    
    // Address: /ch/XX/mix/on
    pos += snprintf((char*)&packet[pos], 30, "/ch/%02d/mix/on", channel + 1);
    pos++;  // null terminator
    
    // Pad to 4-byte boundary
    while (pos % 4 != 0) {
        packet[pos++] = 0;
    }
    
    // Type tag: ,i
    packet[pos++] = ',';
    packet[pos++] = 'i';
    packet[pos++] = 0;
    packet[pos++] = 0;
    
    // Int value (big-endian): 0=muted, 1=unmuted
    int on = muted ? 0 : 1;
    uint32_t int_val = on;
    packet[pos++] = (int_val >> 24) & 0xFF;
    packet[pos++] = (int_val >> 16) & 0xFF;
    packet[pos++] = (int_val >> 8) & 0xFF;
    packet[pos++] = int_val & 0xFF;
    
    osc_send(packet, pos);
    
    if (g_osc_verbose) {
        printf("[OSC] CH%02d mute: %s\n", channel + 1, muted ? "ON" : "OFF");
    }
}

// Send EQ parameter for a channel
void osc_send_eq_param(int channel, int band, const char *param, float value)
{
    // Validate inputs
    if (channel < 0 || channel >= 16) return;
    if (band < 0 || band >= 5) return;
    
    // Build OSC message: /ch/XX/eq/B/param ,f <value>
    // param: "type" (int), "f" (float), "g" (float), "q" (float)
    
    uint8_t packet[40];
    int pos = 0;
    int is_int = (strcmp(param, "type") == 0);
    
    // Address: /ch/XX/eq/B/param
    pos += snprintf((char*)&packet[pos], 38, "/ch/%02d/eq/%d/%s", channel + 1, band + 1, param);
    pos++;  // null terminator
    
    // Pad to 4-byte boundary
    while (pos % 4 != 0) {
        packet[pos++] = 0;
    }
    
    // Type tag: ,i or ,f
    packet[pos++] = ',';
    packet[pos++] = is_int ? 'i' : 'f';
    packet[pos++] = 0;
    packet[pos++] = 0;
    
    // Value (big-endian)
    if (is_int) {
        uint32_t int_val = (uint32_t)value;
        packet[pos++] = (int_val >> 24) & 0xFF;
        packet[pos++] = (int_val >> 16) & 0xFF;
        packet[pos++] = (int_val >> 8) & 0xFF;
        packet[pos++] = int_val & 0xFF;
    } else {
        uint32_t float_bits = *(uint32_t*)&value;
        packet[pos++] = (float_bits >> 24) & 0xFF;
        packet[pos++] = (float_bits >> 16) & 0xFF;
        packet[pos++] = (float_bits >> 8) & 0xFF;
        packet[pos++] = float_bits & 0xFF;
    }
    
    osc_send(packet, pos);
    
    if (g_osc_verbose) {
        printf("[OSC] CH%02d EQ%d %s: %.2f\n", channel + 1, band + 1, param, value);
    }
}

// Shutdown OSC (called on app exit)
void osc_shutdown(void)
{
    if (g_osc_socket >= 0) {
        close(g_osc_socket);
        g_osc_socket = -1;
        g_osc_connected = 0;
        if (g_osc_verbose) printf("[OSC] Connection closed\n");
    }
}

// Send complete step data via OSC (all 16 faders + 16 mutes + all EQ data)
void send_step_osc(int step_idx)
{
    FILE *dbg = fopen("/3ds/x18mixer/osc_debug.txt", "a");
    if (dbg) fprintf(dbg, "[SEND_STEP] step_idx=%d, connected=%d, socket=%d, send_fader=%d, send_eq=%d\n", 
                     step_idx, g_osc_connected, g_osc_socket, g_options.send_fader, g_options.send_eq);
    
    if (step_idx < 0 || step_idx >= g_current_show.num_steps) {
        if (dbg) fprintf(dbg, "[SEND_STEP] Invalid step index\n");
        if (dbg) fclose(dbg);
        return;
    }
    if (!g_osc_connected) {
        if (dbg) fprintf(dbg, "[SEND_STEP] OSC not connected, returning\n");
        if (dbg) fclose(dbg);
        return;
    }
    
    Step *step = &g_current_show.steps[step_idx];
    
    if (g_osc_verbose) {
        printf("[OSC] === Sending Step %d ===\n", step_idx + 1);
    }
    
    // Send all 16 faders (if enabled)
    if (g_options.send_fader) {
        for (int ch = 0; ch < 16; ch++) {
            osc_send_fader(ch, step->volumes[ch]);
        }
    }
    
    // Send all 16 mutes (ALWAYS sent, regardless of options)
    for (int ch = 0; ch < 16; ch++) {
        osc_send_mute(ch, step->mutes[ch]);
    }
    
    // Send all EQ data (5 bands per channel) - if enabled AND channel EQ is enabled
    if (g_options.send_eq) {
        for (int ch = 0; ch < 16; ch++) {
            ChannelEQ *eq = &step->eqs[ch];
            // Only send EQ data if this channel's EQ is enabled
            if (eq->enabled) {
                for (int band = 0; band < 5; band++) {
                    EQBand *eq_band = &eq->bands[band];
                    // Send EQ band type, frequency, gain, Q factor
                    osc_send_eq_param(ch, band, "type", (float)eq_band->type);
                    osc_send_eq_param(ch, band, "f", eq_band->frequency);
                    osc_send_eq_param(ch, band, "g", eq_band->gain);
                    osc_send_eq_param(ch, band, "q", eq_band->q_factor);
                }
            }
        }
    }
    
    if (g_osc_verbose) {
        printf("[OSC] Step %d complete (16 faders + 16 mutes + 80 EQ params)\n", step_idx + 1);
    }
    
    if (dbg) {
        fprintf(dbg, "[SEND_STEP] Step %d sent successfully\n", step_idx);
        fclose(dbg);
    }
}

// ============================================================================
// SHOW/PRESET FUNCTIONS
// ============================================================================


// Initialize EQ with default values
void init_channel_eq(ChannelEQ *eq)
{
    eq->enabled = 0;  // EQ disabled by default
    
    // Band 0: 64Hz Low Cut
    eq->bands[0].frequency = 64.0f;
    eq->bands[0].gain = 0.0f;
    eq->bands[0].q_factor = 0.5f;
    eq->bands[0].type = EQ_LCUT;
    
    // Band 1: 200Hz Peaking EQ
    eq->bands[1].frequency = 200.0f;
    eq->bands[1].gain = 0.0f;
    eq->bands[1].q_factor = 5.0f;
    eq->bands[1].type = EQ_PEQ;
    
    // Band 2: 500Hz Peaking EQ
    eq->bands[2].frequency = 500.0f;
    eq->bands[2].gain = 0.0f;
    eq->bands[2].q_factor = 5.0f;
    eq->bands[2].type = EQ_PEQ;
    
    // Band 3: 1200Hz Peaking EQ
    eq->bands[3].frequency = 1200.0f;
    eq->bands[3].gain = 0.0f;
    eq->bands[3].q_factor = 5.0f;
    eq->bands[3].type = EQ_PEQ;
    
    // Band 4: 4500Hz High Shelf
    eq->bands[4].frequency = 4500.0f;
    eq->bands[4].gain = 0.0f;
    eq->bands[4].q_factor = 0.5f;
    eq->bands[4].type = EQ_HSHV;
}

void init_default_show(void)
{
    // CRITICAL: Initialize entire Show structure to zero FIRST
    memset(&g_current_show, 0, sizeof(Show));
    
    // Magic number for validation
    g_current_show.magic = 0x58334D32;  // "X34M" in hex - version identifier
    
    // Try to load last saved show from persistence
    FILE *f = fopen("/3ds/x18mixer/last_show.txt", "r");
    if (f) {
        char last_show[64] = "";
        if (fgets(last_show, sizeof(last_show), f)) {
            // Remove newline if present
            char *nl = strchr(last_show, '\n');
            if (nl) *nl = '\0';
            
            if (load_show_from_file(last_show, &g_current_show)) {
                fclose(f);
                g_show_loaded = 1;
                g_selected_step = 0;
                g_show_modified = 0;  // Loaded from disk, not modified
                apply_step_to_faders(0);
                return;
            }
        }
        fclose(f);
    }
    
    // If no saved show found, create default
    strcpy(g_current_show.name, "Default Show");
    g_current_show.num_steps = 3;
    
    for (int s = 0; s < 3; s++) {
        snprintf(g_current_show.steps[s].name, sizeof(g_current_show.steps[s].name), "Step %d", s + 1);
        
        for (int i = 0; i < 16; i++) {
            g_current_show.steps[s].volumes[i] = 0.5f;
            g_current_show.steps[s].mutes[i] = 0;
            init_channel_eq(&g_current_show.steps[s].eqs[i]);
        }
    }
    g_show_loaded = 1;
    g_show_modified = 0;  // Default show is not modified
}

void init_new_show(const char *name)
{
    // Initialize a brand new show with default 3 steps
    // CRITICAL: Initialize entire Show structure to zero FIRST
    memset(&g_current_show, 0, sizeof(Show));
    
    // Magic number for validation
    g_current_show.magic = 0x58334D32;
    
    strcpy(g_current_show.name, name);
    g_current_show.num_steps = 3;
    
    for (int s = 0; s < 3; s++) {
        snprintf(g_current_show.steps[s].name, sizeof(g_current_show.steps[s].name), "Step %d", s + 1);
        
        for (int i = 0; i < 16; i++) {
            g_current_show.steps[s].volumes[i] = 0.5f;
            g_current_show.steps[s].mutes[i] = 0;
            init_channel_eq(&g_current_show.steps[s].eqs[i]);
        }
    }
    g_show_loaded = 1;
    g_show_modified = 1;  // New show is modified (not yet saved)
    g_selected_step = 0;
}

void apply_step_to_faders(int step_idx)
{
    // Safety: Protect from uninitialized access
    if (!g_init_complete) return;
    if (step_idx < 0 || step_idx >= g_current_show.num_steps) return;
    
    Step *step = &g_current_show.steps[step_idx];
    for (int i = 0; i < 16; i++) {
        g_faders[i].value = step->volumes[i];
        g_faders[i].muted = step->mutes[i];
        g_faders[i].eq_enabled = step->eqs[i].enabled;  // Get enabled flag from ChannelEQ
    }
    g_selected_step = step_idx;
}

void save_step_from_faders(int step_idx)
{
    if (step_idx < 0 || step_idx >= g_current_show.num_steps) return;
    
    Step *step = &g_current_show.steps[step_idx];
    for (int i = 0; i < 16; i++) {
        step->volumes[i] = g_faders[i].value;
        step->mutes[i] = g_faders[i].muted;
        // NOTE: Do NOT overwrite eq_enabled here - it's managed by the EQ window separately
        // step->eqs[i].enabled is preserved from user's EQ editing
    }
}

void add_step(void)
{
    // Cannot add more than 200 steps
    if (g_current_show.num_steps >= 200) {
        snprintf(g_save_status, sizeof(g_save_status), "Cannot add more than 200 steps");
        g_save_status_timer = 120;
        return;
    }
    
    int new_idx = g_current_show.num_steps;
    Step *new_step = &g_current_show.steps[new_idx];
    
    // Create new step with default values
    snprintf(new_step->name, sizeof(new_step->name), "Step %d", new_idx + 1);
    
    for (int i = 0; i < 16; i++) {
        new_step->volumes[i] = 0.5f;
        new_step->mutes[i] = 0;
        init_channel_eq(&new_step->eqs[i]);
    }
    
    g_current_show.num_steps++;
    g_selected_step = new_idx;
    g_show_modified = 1;  // Mark as modified
    apply_step_to_faders(new_idx);
    
    snprintf(g_save_status, sizeof(g_save_status), "Added Step %d", new_idx + 1);
    g_save_status_timer = 120;
}

void duplicate_step(void)
{
    // Cannot add more than 200 steps
    if (g_current_show.num_steps >= 200) {
        snprintf(g_save_status, sizeof(g_save_status), "Cannot add more than 200 steps");
        g_save_status_timer = 120;
        return;
    }
    
    // Get current step
    if (g_selected_step < 0 || g_selected_step >= g_current_show.num_steps) {
        snprintf(g_save_status, sizeof(g_save_status), "Invalid step to duplicate");
        g_save_status_timer = 120;
        return;
    }
    
    Step *src_step = &g_current_show.steps[g_selected_step];
    int new_idx = g_current_show.num_steps;
    Step *new_step = &g_current_show.steps[new_idx];
    
    // Copy the step
    memcpy(new_step, src_step, sizeof(Step));
    
    // Build name safely using temp buffer
    char temp_name[64];
    snprintf(temp_name, sizeof(temp_name), "%s copy", src_step->name);
    strcpy(new_step->name, temp_name);
    
    g_current_show.num_steps++;
    g_selected_step = new_idx;
    g_show_modified = 1;  // Mark as modified
    apply_step_to_faders(new_idx);
    
    snprintf(g_save_status, sizeof(g_save_status), "Duplicated Step %d", new_idx + 1);
    g_save_status_timer = 120;
}

// ============================================================================
// SHOW MANAGER FILE I/O
// ============================================================================


// Test filesystem write capability
int test_filesystem_write(void)
{
    create_shows_directory();
    
    char test_path[256];
    snprintf(test_path, sizeof(test_path), "%stest.txt", SHOWS_DIR);
    
    FILE *f = fopen(test_path, "wb");
    if (!f) {
        snprintf(g_save_status, sizeof(g_save_status), "ERROR: Cannot open file at %s", SHOWS_DIR);
        return 0;
    }
    
    // Try to write test data
    const char *test_data = "TEST";
    size_t written = fwrite(test_data, 1, 4, f);
    fflush(f);
    fsync(fileno(f));
    fclose(f);
    
    if (written != 4) {
        snprintf(g_save_status, sizeof(g_save_status), "ERROR: Write failed (wrote %zu bytes)", written);
        return 0;
    }
    
    snprintf(g_save_status, sizeof(g_save_status), "OK: Filesystem test passed! Saved to %s", SHOWS_DIR);
    return 1;
}

// Sanitize filename - replace spaces and invalid chars with underscores
void sanitize_filename(const char *input, char *output, int max_len)
{
    if (!input || !output) return;
    
    int i = 0;
    for (i = 0; i < max_len - 1 && input[i] != '\0'; i++) {
        char c = input[i];
        // Allow alphanumerics, underscore, hyphen, dot
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || 
            (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.') {
            output[i] = c;
        } else {
            // Replace spaces and other chars with underscore
            output[i] = '_';
        }
    }
    output[i] = '\0';
}

void save_show_to_file(Show *show)
{
    if (!show) return;
    
    create_shows_directory();
    
    // Ensure magic number is set BEFORE saving
    if (show->magic != 0x58334D32) {
        show->magic = 0x58334D32;
    }
    
    // Sanitize the show name for use as a filename
    char safe_name[64];
    sanitize_filename(show->name, safe_name, sizeof(safe_name));
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s%s.x18s", SHOWS_DIR, safe_name);
    
    // Try to open file for writing
    FILE *f = fopen(filepath, "wb");
    if (!f) {
        snprintf(g_save_status, sizeof(g_save_status), "ERROR: Cannot save file");
        g_save_status_timer = 120;  // Show for 2 seconds
        return;
    }
    
    // Write the show struct
    size_t written = fwrite(show, sizeof(Show), 1, f);
    
    // Ensure data is written
    fflush(f);
    fsync(fileno(f));
    fclose(f);
    
    // Report status
    if (written == 1) {
        snprintf(g_save_status, sizeof(g_save_status), "SAVED: %s (%d steps)", safe_name, show->num_steps);
        g_save_status_timer = 120;  // Show for 2 seconds
        
        // Update last show persistence file
        FILE *pf = fopen("/3ds/x18mixer/last_show.txt", "w");
        if (pf) {
            fprintf(pf, "%s", safe_name);
            fflush(pf);
            fsync(fileno(pf));
            fclose(pf);
        }
        
        g_show_modified = 0;  // Mark as saved
    } else {
        snprintf(g_save_status, sizeof(g_save_status), "ERROR: Write failed for %s", safe_name);
        g_save_status_timer = 120;
    }
}

// Save only the EQ for a specific channel in the current step
// Simply saves the entire current show (all EQ data for all channels)
// The user is only modifying EQ, so this is safe and reliable
void save_channel_eq_only(int channel)
{
    if (channel < 0 || channel >= 16) return;
    if (g_selected_step < 0 || g_selected_step >= g_current_show.num_steps) return;
    
    // Directly save the entire show to ensure consistency
    // All EQ data (including this channel) will be persisted
    save_show_to_file(&g_current_show);
    
    snprintf(g_save_status, sizeof(g_save_status), "CH %02d EQ saved", channel + 1);
    g_save_status_timer = 120;
}

// Old format structures (for backward compatibility)
typedef struct {
    char name[32];
    float volumes[16];
    int mutes[16];
    int eqs_old[16];  // OLD: was just an int flag
} __attribute__((packed)) OldStep;

typedef struct {
    char name[64];
    OldStep steps_old[200];
    int num_steps;
} __attribute__((packed)) OldShow;

// Convert old show format to new format
void migrate_old_show_to_new(OldShow *old, Show *new_show)
{
    // CRITICAL: Initialize entire structure to zero first
    memset(new_show, 0, sizeof(Show));
    
    strcpy(new_show->name, old->name);
    new_show->num_steps = old->num_steps;
    if (new_show->num_steps > 200) new_show->num_steps = 200;  // Sanity check
    if (new_show->num_steps < 1) new_show->num_steps = 1;      // At least 1 step
    
    for (int s = 0; s < new_show->num_steps; s++) {
        strcpy(new_show->steps[s].name, old->steps_old[s].name);
        
        for (int i = 0; i < 16; i++) {
            new_show->steps[s].volumes[i] = old->steps_old[s].volumes[i];
            new_show->steps[s].mutes[i] = old->steps_old[s].mutes[i];
            
            // Initialize new EQ structure
            init_channel_eq(&new_show->steps[s].eqs[i]);
            // Set enabled flag from old format
            new_show->steps[s].eqs[i].enabled = old->steps_old[s].eqs_old[i];
        }
    }
}

int load_show_from_file(const char *filename, Show *out_show)
{
    if (!filename || !out_show) return 0;
    
    create_shows_directory();
    
    // CRITICAL: Initialize entire Show structure to zero BEFORE loading
    memset(out_show, 0, sizeof(Show));
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s%s.x18s", SHOWS_DIR, filename);
    
    FILE *f = fopen(filepath, "rb");
    if (!f) return 0;
    
    // Get file size to detect format version
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // Calculate expected size
    long expected_new_size = sizeof(Show);
    
    // Old format size: ~44868 bytes (64 + 200*224 + 4)
    // New format size: ~300868 bytes (64 + 200*1504 + 4)
    
    // Check if this looks like old format by file size
    if (file_size < 100000) {  // Conservatively assume < 100KB is old format
        // Allocate old show on HEAP to avoid stack overflow
        OldShow *old_show = (OldShow*)malloc(sizeof(OldShow));
        if (!old_show) {
            fclose(f);
            return 0;
        }
        
        memset(old_show, 0, sizeof(OldShow));
        size_t read = fread(old_show, sizeof(OldShow), 1, f);
        fclose(f);
        
        if (read == 1 && old_show->num_steps > 0 && old_show->num_steps <= 200) {
            // Successfully loaded as old format, migrate to new
            migrate_old_show_to_new(old_show, out_show);
            free(old_show);
            return 1;
        }
        // If migration failed, keep the zero-initialized structure
        free(old_show);
        return 0;
    }
    
    // Load as new format - but first verify file size matches expected
    if (file_size != expected_new_size) {
        // File size mismatch - likely corrupted or compiled with different struct sizes
        fclose(f);
        return 0;
    }
    
    size_t read = fread(out_show, sizeof(Show), 1, f);
    fclose(f);
    
    // Verify loaded data is sane
    if (read != 1) return 0;
    
    // Comprehensive validation of loaded data
    if (out_show->num_steps < 1 || out_show->num_steps > 200) {
        // Invalid step count
        memset(out_show, 0, sizeof(Show));
        return 0;
    }
    
    // Check magic number - but be tolerant of old files (magic == 0)
    // Only fail if magic is non-zero AND not our expected value
    if (out_show->magic != 0 && out_show->magic != 0x58334D32) {
        // File header is corrupted
        memset(out_show, 0, sizeof(Show));
        return 0;
    }
    
    // If magic is 0, set it now (was an old file)
    if (out_show->magic == 0) {
        out_show->magic = 0x58334D32;
    }
    
    // Additional sanity checks on first step
    Step *first_step = &out_show->steps[0];
    
    // Check that volumes are in valid range
    for (int i = 0; i < 16; i++) {
        if (first_step->volumes[i] < 0.0f || first_step->volumes[i] > 1.0f) {
            // Volume out of range - data corrupted
            memset(out_show, 0, sizeof(Show));
            return 0;
        }
        // Check that mutes are 0 or 1
        if (first_step->mutes[i] != 0 && first_step->mutes[i] != 1) {
            // Invalid mute value
            memset(out_show, 0, sizeof(Show));
            return 0;
        }
    }
    
    return 1;
}

void list_available_shows(void)
{
    g_num_available_shows = 0;
    create_shows_directory();
    
    DIR *dir = opendir(SHOWS_DIR);
    if (!dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) && g_num_available_shows < MAX_SHOWS) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".x18s")) {
            // Remove .x18s extension
            strcpy(g_available_shows[g_num_available_shows], entry->d_name);
            char *dot = strstr(g_available_shows[g_num_available_shows], ".x18s");
            if (dot) *dot = '\0';
            g_num_available_shows++;
        }
    }
    closedir(dir);
}

// Load network configuration from file
void load_network_config(void)
{
    FILE *dbg = fopen("/3ds/x18mixer/osc_debug.txt", "a");
    if (dbg) fprintf(dbg, "[LOAD_CONFIG] Loading network config...\n");
    
    FILE *f = fopen("/3ds/x18mixer/net.txt", "r");
    if (!f) {
        // Use defaults if file doesn't exist
        if (dbg) fprintf(dbg, "[LOAD_CONFIG] File not found, using defaults\n");
        strcpy(g_net_ip_digits, "10101099112");
        strcpy(g_mixer_host, "10.10.99.112");
        snprintf(g_net_port_input, sizeof(g_net_port_input), "%d", g_mixer_port);
        if (dbg) fprintf(dbg, "[LOAD_CONFIG] Defaults: host='%s', port=%d\n", g_mixer_host, g_mixer_port);
        if (dbg) fclose(dbg);
        return;
    }
    
    if (dbg) fprintf(dbg, "[LOAD_CONFIG] File found, reading...\n");
    
    // Parse config file: "IP PORT"
    char line[256];
    if (fgets(line, sizeof(line), f)) {
        char ip[16];
        int port;
        int parsed = sscanf(line, "%15s %d", ip, &port);
        if (dbg) fprintf(dbg, "[LOAD_CONFIG] Read line: '%s', parsed=%d items\n", line, parsed);
        
        if (parsed == 2) {
            // Validate IP by trying to parse it with inet_pton
            struct in_addr temp;
            int valid = inet_pton(AF_INET, ip, &temp);
            if (dbg) fprintf(dbg, "[LOAD_CONFIG] inet_pton validation: %d for IP '%s'\n", valid, ip);
            
            if (valid > 0) {
                // Valid IP format - use it
                strcpy(g_mixer_host, ip);
                g_mixer_port = port;
                
                // Also update digit representation
                int digit_idx = 0;
                for (int i = 0; ip[i] != '\0' && digit_idx < 12; i++) {
                    if (isdigit((unsigned char)ip[i])) {
                        g_net_ip_digits[digit_idx++] = ip[i];
                    }
                }
                g_net_ip_digits[digit_idx] = '\0';  // Null terminate
                
                if (dbg) fprintf(dbg, "[LOAD_CONFIG] Loaded valid: host='%s', port=%d\n", g_mixer_host, g_mixer_port);
            } else {
                // Invalid IP - use defaults
                if (dbg) fprintf(dbg, "[LOAD_CONFIG] Invalid IP format, using defaults\n");
                strcpy(g_net_ip_digits, "10101099112");
                strcpy(g_mixer_host, "10.10.99.112");
                snprintf(g_net_port_input, sizeof(g_net_port_input), "%d", g_mixer_port);
            }
        } else {
            if (dbg) fprintf(dbg, "[LOAD_CONFIG] Failed to parse, using defaults\n");
            strcpy(g_net_ip_digits, "10101099112");
            strcpy(g_mixer_host, "10.10.99.112");
            snprintf(g_net_port_input, sizeof(g_net_port_input), "%d", g_mixer_port);
        }
    }
    fclose(f);
    if (dbg) fclose(dbg);
}

// Save network configuration to file
void save_network_config(void)
{
    create_shows_directory();
    
    FILE *f = fopen("/3ds/x18mixer/net.txt", "w");
    if (!f) {
        snprintf(g_save_status, sizeof(g_save_status), "ERROR: Cannot save net.txt");
        g_save_status_timer = 120;
        return;
    }
    
    // Parse and validate port
    int port = atoi(g_net_port_input);
    if (port <= 0 || port > 65535) {
        port = 10023;  // Default
    }
    
    // Convert 12 digits to proper IP format (without leading zeros)
    // g_net_ip_digits = "101099112" (9 digits) -> need to pad to 12: "000101099112"
    // Then format as: octet1.octet2.octet3.octet4
    char padded[13] = {0};
    int len = strlen(g_net_ip_digits);
    int pad_count = 12 - len;
    for (int i = 0; i < pad_count; i++) padded[i] = '0';
    strcpy(padded + pad_count, g_net_ip_digits);
    
    // Extract 4 octets as integers (removes leading zeros automatically)
    int oct1 = ((padded[0]-'0')*100 + (padded[1]-'0')*10 + (padded[2]-'0'));
    int oct2 = ((padded[3]-'0')*100 + (padded[4]-'0')*10 + (padded[5]-'0'));
    int oct3 = ((padded[6]-'0')*100 + (padded[7]-'0')*10 + (padded[8]-'0'));
    int oct4 = ((padded[9]-'0')*100 + (padded[10]-'0')*10 + (padded[11]-'0'));
    
    // Write as proper IP without leading zeros
    char ip_proper[20];
    snprintf(ip_proper, sizeof(ip_proper), "%d.%d.%d.%d", oct1, oct2, oct3, oct4);
    fprintf(f, "%s %d\n", ip_proper, port);
    
    // Also update the global config
    strcpy(g_mixer_host, ip_proper);
    g_mixer_port = port;
    
    fflush(f);
    fsync(fileno(f));
    fclose(f);
    
    snprintf(g_save_status, sizeof(g_save_status), "Network config saved: %s:%d", g_mixer_host, g_mixer_port);
    g_save_status_timer = 120;
}

void delete_show_file(const char *filename)
{
    if (!filename) return;
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s%s.x18s", SHOWS_DIR, filename);
    unlink(filepath);
    list_available_shows();
}

void duplicate_show_file(const char *src, const char *dst)
{
    if (!src || !dst) return;
    
    Show temp_show;
    if (!load_show_from_file(src, &temp_show)) return;
    
    strcpy(temp_show.name, dst);
    save_show_to_file(&temp_show);
    list_available_shows();
}

void rename_show_file(const char *old_name, const char *new_name)
{
    if (!old_name || !new_name) return;
    
    Show temp_show;
    if (!load_show_from_file(old_name, &temp_show)) return;
    
    delete_show_file(old_name);
    strcpy(temp_show.name, new_name);
    save_show_to_file(&temp_show);
    list_available_shows();
}

// ============================================================================
// MIXER FUNCTIONS
// ============================================================================

void init_mixer(void)
{
    for (int i = 0; i < NUM_FADERS; i++) {
        g_faders[i].id = i + 1;
        g_faders[i].value = 0.5f;
        g_faders[i].muted = 0;
        g_faders[i].eq_enabled = 0;
        g_faders[i].x = i * FADER_WIDTH;
        g_faders[i].y = 0;
        g_faders[i].w = FADER_WIDTH;
        g_faders[i].h = FADER_HEIGHT;
    }
}

int touch_hits_fader(touchPosition touch, Fader *fader, float *out_value)
{
    if (touch.px >= fader->x && touch.px < fader->x + fader->w) {
        // Grip movement range: matches the actual grip Y positions in render_bot_screen
        // fader_bottom=205, fader_height=160, so grip goes from:
        // value=1: grip_y = 205 - (160*1.0) = 45
        // value=0: grip_y = 205 - (160*0.15) = 181
        float fader_top = 45;      // Grip top position (value = 1.0)
        float fader_bottom = 181;  // Grip bottom position (value = 0.0)
        float grip_half_height = 8.0f;  // Grip is 16px tall, center offset = 8px
        
        if (touch.py >= fader_top - grip_half_height && touch.py <= fader_bottom + grip_half_height) {
            // Calculate based on grip CENTER position, not touch point
            float grip_center_from_fader_top = (touch.py - grip_half_height) - fader_top;
            float fader_height = fader_bottom - fader_top;
            float val = 1.0f - (grip_center_from_fader_top / fader_height);
            *out_value = (val < 0) ? 0 : (val > 1) ? 1 : val;
            return 1;
        }
    }
    return 0;
}

int touch_hits_mute_button(touchPosition touch, Fader *fader)
{
    return (touch.px >= fader->x && touch.px < fader->x + fader->w &&
            touch.py >= 210 && touch.py <= 227);
}

int touch_hits_eq_button(touchPosition touch, Fader *fader)
{
    return (touch.px >= fader->x && touch.px < fader->x + fader->w &&
            touch.py >= 5 && touch.py <= 22);
}

// Forward declarations for touch handlers
void update_eq_touch(void);

void update_mixer_touch(void)
{
    int touch_edge = g_isTouched && !g_wasTouched;
    
    // If touch ended, reset the touched fader tracking
    if (!g_isTouched) {
        g_touched_fader_index = -1;
        return;
    }
    
    // If EQ window is open, handle EQ touch input instead
    if (g_eq_window_open) {
        update_eq_touch();
        return;
    }
    
    // On new touch, find which fader is being touched
    if (touch_edge) {
        g_touched_fader_index = -1;  // Reset
        
        for (int i = 0; i < NUM_FADERS; i++) {
            float val;
            
            // Check fader first (highest priority - drag interaction)
            if (touch_hits_fader(g_touchPos, &g_faders[i], &val)) {
                g_faders[i].value = val;
                g_touched_fader_index = i;  // Mark this fader as being touched
                return;  // Stop checking other faders
            }
            
            // Check mute button
            if (touch_hits_mute_button(g_touchPos, &g_faders[i])) {
                g_faders[i].muted = 1 - g_faders[i].muted;
                return;  // Stop checking other faders
            }
            
            // Check EQ button
            if (touch_hits_eq_button(g_touchPos, &g_faders[i])) {
                g_eq_window_open = 1;
                g_eq_editing_channel = i;
                g_eq_selected_band = 0;
                g_eq_param_selected = 0;
                return;  // Stop checking other faders
            }
        }
    }
    
    // While touching, continue updating only the tracked fader
    if (g_touched_fader_index >= 0 && g_touched_fader_index < NUM_FADERS) {
        float val;
        if (touch_hits_fader(g_touchPos, &g_faders[g_touched_fader_index], &val)) {
            g_faders[g_touched_fader_index].value = val;
        }
    }
}

// Handle touch input in EQ window


void update_touch_input(void)
{
    hidTouchRead(&g_touchPos);
    g_wasTouched = g_isTouched;
    g_isTouched = (hidKeysHeld() & KEY_TOUCH) ? 1 : 0;
    update_mixer_touch();
}

// ============================================================================
// GRAPHICS FUNCTIONS
// ============================================================================

// Forward declarations
void render_eq_window(void);

// Helper function to get filter type name

void init_graphics(void)
{
    gfxInitDefault();
    
    // Initialize file system service - CRITICAL for CIA to access /sdmc/
    // Without this, SD card access fails with "SD card was removed" error
    // The .3dsx launcher does this automatically, but CIA needs it explicitly
    fsInit();
    
    // Initialize socket services on 3DS BEFORE using sockets
    int soc_ret = socInit(SOC_BUFFER, sizeof(SOC_BUFFER));
    FILE *dbg_soc = fopen("/3ds/x18mixer/osc_debug.txt", "w");
    if (dbg_soc) {
        fprintf(dbg_soc, "[INIT] socInit() returned: %d\n", soc_ret);
        fclose(dbg_soc);
    }
    
    // Initialize OSC (Phase 1)
    osc_init();
    
    // Load network configuration
    load_network_config();
    
    // Load OSC send options
    init_options();
    load_options();
    
    // Mount RomFS for loading embedded assets (required before accessing romfs:/)
    romfsInit();
    
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    
    g_topScreen.target = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    g_topScreen.width = SCREEN_WIDTH_TOP;
    g_topScreen.height = SCREEN_HEIGHT_TOP;
    
    g_botScreen.target = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    g_botScreen.width = SCREEN_WIDTH_BOT;
    g_botScreen.height = SCREEN_HEIGHT_BOT;
    
    g_textBuf = C2D_TextBufNew(2048);
    
    // Load system font for better text rendering (instead of bitmap fonts)
    g_font = C2D_FontLoadSystem(CFG_REGION_USA);
    if (!g_font) {
        printf("[WARNING] Failed to load system font, will use default font\n");
    } else {
        printf("[DEBUG] System font loaded successfully\n");
    }
    
    // Ensure sprite sheets are available on SD card (for CIA compatibility)
    ensure_sprite_sheets_on_sd();
    
    // Load fader sprite sheets from romfs (for .3dsx) or /3ds/x18mixer/gfx/ (for CIA on SD card)
    printf("[DEBUG] Attempting to load spritesheets from romfs:/ or SD card\n");
    
    // Try both possible paths: romfs:/gfx/ and romfs:/ (3dsxtool might put files in root)
    g_grip_sheet = C2D_SpriteSheetLoad("romfs:/Grip.t3x");
    if (!g_grip_sheet) {
        g_grip_sheet = C2D_SpriteSheetLoad("romfs:/gfx/Grip.t3x");
    }
    // If RomFS failed, try loading from SD card (CIA doesn't embed RomFS by default)
    if (!g_grip_sheet) {
        g_grip_sheet = C2D_SpriteSheetLoad("/3ds/x18mixer/gfx/Grip.t3x");
    }
    if (g_grip_sheet) {
        g_grip_img = C2D_SpriteSheetGetImage(g_grip_sheet, 0);
        g_grip_loaded = 1;
        printf("[DEBUG] Successfully loaded Grip.t3x\n");
    } else {
        printf("[DEBUG] Failed to load Grip.t3x from all paths\n");
    }
    
    // Load fader background sprite sheet
    g_fader_sheet = C2D_SpriteSheetLoad("romfs:/FaderBkg.t3x");
    if (!g_fader_sheet) {
        g_fader_sheet = C2D_SpriteSheetLoad("romfs:/gfx/FaderBkg.t3x");
    }
    // If RomFS failed, try loading from SD card
    if (!g_fader_sheet) {
        g_fader_sheet = C2D_SpriteSheetLoad("romfs:/gfx/FaderBkg.t3x");
    }
    // If RomFS failed, try loading from SD card
    if (!g_fader_sheet) {
        g_fader_sheet = C2D_SpriteSheetLoad("/3ds/x18mixer/gfx/FaderBkg.t3x");
    }
    if (g_fader_sheet) {
        g_fader_bkg = C2D_SpriteSheetGetImage(g_fader_sheet, 0);
        g_fader_loaded = 1;
        printf("[DEBUG] Successfully loaded FaderBkg.t3x\n");
    } else {
        printf("[DEBUG] Failed to load FaderBkg.t3x from all paths\n");
    }
    
    g_romfs_mounted = 1;
    
    init_mixer();
    init_default_show();
    
    // Test filesystem on startup
    test_filesystem_write();
    g_save_status_timer = 180;  // Show for 3 seconds on startup
    
    // CRITICAL: Mark initialization as complete - rendering is now safe
    // Must be set BEFORE calling apply_step_to_faders
    g_init_complete = 1;
    
    // Now safe to apply step to faders
    apply_step_to_faders(0);
}

void cleanup_graphics(void)
{
    // Free sprite sheets
    if (g_grip_sheet) {
        C2D_SpriteSheetFree(g_grip_sheet);
        g_grip_sheet = NULL;
    }
    if (g_fader_sheet) {
        C2D_SpriteSheetFree(g_fader_sheet);
        g_fader_sheet = NULL;
    }
    
    // Free font
    if (g_font) {
        C2D_FontFree(g_font);
        g_font = NULL;
    }
    
    if (g_textBuf) {
        C2D_TextBufDelete(g_textBuf);
        g_textBuf = NULL;
    }
    C2D_Fini();
    C3D_Fini();
    
    // Shutdown OSC (Phase 1)
    osc_shutdown();
    
    // Unmount RomFS
    romfsExit();
    
    // Shutdown file system service
    fsExit();
    
    gfxExit();
}

// NOTE: draw_rect not used - removed

// Draw a 3D button with shadow effect
void draw_3d_button(float x, float y, float w, float h, u32 color_main, u32 color_light, u32 color_dark, int pressed)
{
    // Shadow (bottom-right)
    C2D_DrawRectSolid(x + 2, y + 2, 0.4f, w, h, C2D_Color32(0x00, 0x00, 0x00, 0x60));
    
    // Main button body
    C2D_DrawRectSolid(x, y, 0.5f, w, h, color_main);
    
    if (!pressed) {
        // Top-left highlight
        C2D_DrawRectSolid(x, y, 0.51f, w - 1, 1, color_light);
        C2D_DrawRectSolid(x, y, 0.51f, 1, h - 1, color_light);
        
        // Bottom-right shadow
        C2D_DrawRectSolid(x + w - 1, y, 0.51f, 1, h, color_dark);
        C2D_DrawRectSolid(x, y + h - 1, 0.51f, w, 1, color_dark);
    } else {
        // Pressed: inverted highlights
        C2D_DrawRectSolid(x + w - 1, y, 0.51f, 1, h, color_light);
        C2D_DrawRectSolid(x, y + h - 1, 0.51f, w, 1, color_light);
    }
}

// ============================================================================
// RENDERING
// ============================================================================


// Calculate EQ response at a specific frequency for a single band

// Draw EQ curve and graph on bottom screen with touch controls


// Convert fader value (0-1) to dB using calibration points



// Helper function to format IP address from digits (xxxxx -> xxx.xxx.xxx.xxx)
// Pads with zeros on left if needed: "101099112" -> "000.101.099.112"

// Render network configuration window (on top screen)



// Forward declaration for keyboard input handling
void handle_keyboard_input(char c);



// Handle input for network configuration window



// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char* argv[])
{
    init_graphics();
    
    while (aptMainLoop())
    {
        hidScanInput();
        update_touch_input();
        
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();
        
        // If EQ window is open, handle EQ input instead of normal controls
        if (g_eq_window_open) {
            handle_eq_input(kDown, kHeld);
        } else if (g_options_window_open) {
            // If options window is open, handle options input
            handle_options_input(kDown);
        } else {
            // Normal mixer/manager controls
            // START button opens/closes show manager
            if (kDown & KEY_START) {
                if (g_app_mode == APP_MODE_MIXER) {
                    // Opening manager: do NOT save, just switch
                    list_available_shows();
                    g_app_mode = APP_MODE_MANAGER;
                } else {
                    // Closing manager: return to mixer
                    g_app_mode = APP_MODE_MIXER;
                }
            }
            
            if (g_app_mode == APP_MODE_MIXER) {
                if (g_creating_new_show) {
                    // Handle new show naming input
                    handle_new_show_input();
                } else {
                    // A button: Send current step OSC data and advance to next step
                    if (kDown & KEY_A) {
                        send_step_osc(g_selected_step);
                        g_selected_step = (g_selected_step + 1) % g_current_show.num_steps;
                        apply_step_to_faders(g_selected_step);
                    }
                    
                    // SELECT button: Start creating new show
                    if (kDown & KEY_SELECT) {
                        g_creating_new_show = 1;
                        g_new_show_input_pos = 0;
                        memset(g_new_show_name, 0, sizeof(g_new_show_name));
                    }
                    
                    // Up/Down: Navigate steps
                    if (kDown & KEY_DUP) {
                        g_selected_step--;
                        if (g_selected_step < 0) g_selected_step = g_current_show.num_steps - 1;
                        apply_step_to_faders(g_selected_step);
                    }
                    if (kDown & KEY_DDOWN) {
                        g_selected_step++;
                        if (g_selected_step >= g_current_show.num_steps) g_selected_step = 0;
                        apply_step_to_faders(g_selected_step);
                    }
                    
                    // X: Save show
                    if (kDown & KEY_X) {
                        save_show_to_file(&g_current_show);
                    }
                    
                    // Y: Save step
                    if (kDown & KEY_Y) {
                        save_step_from_faders(g_selected_step);
                        save_show_to_file(&g_current_show);
                    }
                    
                    // L: Add new step
                    if (kDown & KEY_L) {
                        add_step();
                    }
                    
                    // R: Duplicate current step
                    if (kDown & KEY_R) {
                        duplicate_step();
                    }
                }
            } else {
                // Show Manager mode
                handle_manager_input();
            }
        }
        
        render_frame();
        gspWaitForVBlank();
        
        // Check if we should exit the app
        if (g_should_exit) {
            break;
        }
    }
    
    cleanup_graphics();
    return 0;}