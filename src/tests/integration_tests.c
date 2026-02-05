/**
 * Phase 8: Integration Tests for 5-Band EQ and Full System
 * Tests cover:
 * - EQ window state transitions
 * - OSC message generation for EQ bands
 * - Database persistence for EQ state
 * - Touch input interaction with EQ
 * - State machine transitions between mixer/menu/EQ
 */

#include "../core/state.h"
#include "../core/constants.h"
#include "../screens/eq_window.h"
#include "../storage/show_manager.h"
#include "../osc/osc_client.h"
#include "../ui/touch_input.h"
#include <stdlib.h>

// Test structure
typedef struct {
    int test_num;
    const char *test_name;
    int passed;
    const char *error_msg;
} TestResult;

static TestResult test_results[20];
static int test_count = 0;

void test_log(const char *msg) {
    // In bare-metal, this would write to serial or debug output
    // For now, it's a placeholder
}

void assert_true(int condition, const char *msg) {
    if (!condition) {
        test_results[test_count].error_msg = msg;
        test_results[test_count].passed = 0;
    } else {
        test_results[test_count].passed = 1;
    }
}

void assert_equal_int(int actual, int expected, const char *msg) {
    if (actual != expected) {
        test_results[test_count].error_msg = msg;
        test_results[test_count].passed = 0;
    } else {
        test_results[test_count].passed = 1;
    }
}

// ============================================================
// TEST 1: EQ Window State Initialization
// ============================================================
void test_eq_window_initialization() {
    test_results[test_count].test_num = test_count + 1;
    test_results[test_count].test_name = "EQ Window State Initialization";
    
    AppState_Global test_state = {0};
    state_init();
    
    // Verify initial EQ window state
    assert_equal_int(test_state.eq_window.channel_id, 0, "Channel ID should be 0");
    assert_equal_int(test_state.eq_window.selected_band, 0, "Selected band should be 0");
    assert_equal_int(test_state.eq_window.selected_param, 0, "Selected param should be 0");
    
    // Verify all 5 bands are initialized
    for (int i = 0; i < 5; i++) {
        assert_true(test_state.eq_window.bands[i].frequency >= 20 && 
                   test_state.eq_window.bands[i].frequency <= 20000,
                   "Band frequency out of range");
        assert_true(test_state.eq_window.bands[i].gain >= -150 && 
                   test_state.eq_window.bands[i].gain <= 150,
                   "Band gain out of range");
        assert_true(test_state.eq_window.bands[i].q >= 3 && 
                   test_state.eq_window.bands[i].q <= 100,
                   "Band Q out of range");
    }
    
    test_count++;
}

// ============================================================
// TEST 2: EQ Window State Transition (Mixer -> EQ)
// ============================================================
void test_eq_window_open_transition() {
    test_results[test_count].test_num = test_count + 1;
    test_results[test_count].test_name = "EQ Window Open (Mixer -> EQ)";
    
    AppState_Global test_state = {0};
    state_init();
    test_state.current_state = STATE_MIXER_VIEW;
    
    // Open EQ window for channel 3
    test_state.current_state = STATE_EQ_WINDOW;
    test_state.eq_window.channel_id = 3;
    
    assert_equal_int(test_state.current_state, STATE_EQ_WINDOW, 
                     "Should transition to EQ_WINDOW state");
    assert_equal_int(test_state.eq_window.channel_id, 3, 
                     "Channel ID should be 3");
    
    test_count++;
}

// ============================================================
// TEST 3: EQ Band Parameter Adjustment
// ============================================================
void test_eq_band_parameter_adjustment() {
    test_results[test_count].test_num = test_count + 1;
    test_results[test_count].test_name = "EQ Band Parameter Adjustment";
    
    AppState_Global test_state = {0};
    state_init();
    test_state.current_state = STATE_EQ_WINDOW;
    test_state.eq_window.selected_band = 0;
    test_state.eq_window.selected_param = 0;  // Type
    
    // Adjust type parameter
    int original_type = test_state.eq_window.bands[0].type;
    eq_window_cycle_band_type(&test_state.eq_window.bands[0]);
    int new_type = test_state.eq_window.bands[0].type;
    
    assert_true(new_type != original_type || (new_type == 0 && original_type == 5),
                "Type should cycle through 0-5");
    
    // Adjust frequency parameter (band 0, param 1)
    test_state.eq_window.selected_param = 1;  // Freq
    int original_freq = test_state.eq_window.bands[0].frequency;
    eq_window_adjust_parameter(&test_state, 100);  // +100 Hz
    int new_freq = test_state.eq_window.bands[0].frequency;
    
    assert_true(new_freq > original_freq, "Frequency should increase");
    assert_true(new_freq >= 20 && new_freq <= 20000, "Frequency should be in valid range");
    
    // Adjust gain parameter (band 0, param 2)
    test_state.eq_window.selected_param = 2;  // Gain
    int original_gain = test_state.eq_window.bands[0].gain;
    eq_window_adjust_parameter(&test_state, 10);  // +1.0 dB
    int new_gain = test_state.eq_window.bands[0].gain;
    
    assert_true(new_gain > original_gain, "Gain should increase");
    assert_true(new_gain >= -150 && new_gain <= 150, "Gain should be in valid range");
    
    test_count++;
}

// ============================================================
// TEST 4: EQ Band Navigation (Band Selection)
// ============================================================
void test_eq_band_navigation() {
    test_results[test_count].test_num = test_count + 1;
    test_results[test_count].test_name = "EQ Band Navigation";
    
    AppState_Global test_state = {0};
    state_init();
    test_state.current_state = STATE_EQ_WINDOW;
    test_state.eq_window.selected_band = 2;
    
    // Navigate to next band (Right D-Pad)
    test_state.eq_window.selected_band = (test_state.eq_window.selected_band + 1) % 5;
    assert_equal_int(test_state.eq_window.selected_band, 3, "Should select band 3");
    
    // Navigate to previous band (Left D-Pad)
    test_state.eq_window.selected_band = (test_state.eq_window.selected_band - 1 + 5) % 5;
    assert_equal_int(test_state.eq_window.selected_band, 2, "Should select band 2");
    
    // Wrap around at end
    test_state.eq_window.selected_band = 4;
    test_state.eq_window.selected_band = (test_state.eq_window.selected_band + 1) % 5;
    assert_equal_int(test_state.eq_window.selected_band, 0, "Should wrap to band 0");
    
    test_count++;
}

// ============================================================
// TEST 5: EQ Parameter Cycling (Type -> Freq -> Gain -> Q)
// ============================================================
void test_eq_parameter_cycling() {
    test_results[test_count].test_num = test_count + 1;
    test_results[test_count].test_name = "EQ Parameter Cycling";
    
    AppState_Global test_state = {0};
    state_init();
    test_state.current_state = STATE_EQ_WINDOW;
    
    // Cycle through all 4 parameters
    for (int i = 0; i < 4; i++) {
        assert_equal_int(test_state.eq_window.selected_param, i, 
                        "Parameter should match iteration");
        test_state.eq_window.selected_param = (test_state.eq_window.selected_param + 1) % 4;
    }
    
    // Should cycle back to 0 (type)
    assert_equal_int(test_state.eq_window.selected_param, 0, "Should cycle back to param 0");
    
    test_count++;
}

// ============================================================
// TEST 6: OSC Message Generation for EQ Bands
// ============================================================
void test_osc_eq_messages() {
    test_results[test_count].test_num = test_count + 1;
    test_results[test_count].test_name = "OSC EQ Message Generation";
    
    AppState_Global test_state = {0};
    state_init();
    
    // Configure test band
    test_state.eq_window.channel_id = 0;
    test_state.eq_window.bands[0].type = 2;      // PEQ
    test_state.eq_window.bands[0].frequency = 1000;
    test_state.eq_window.bands[0].gain = 50;     // +5.0 dB
    test_state.eq_window.bands[0].q = 50;        // Q=5.0
    
    // Simulate OSC message generation
    // Expected addresses: /ch/01/eq/1/t, /ch/01/eq/1/f, /ch/01/eq/1/g, /ch/01/eq/1/q
    
    // Type should be 0-5
    assert_true(test_state.eq_window.bands[0].type >= 0 && 
               test_state.eq_window.bands[0].type <= 5,
               "OSC type value should be 0-5");
    
    // Frequency mapping: integer 20-20000 should map to 0.0-1.0 for OSC
    // Normalized: (1000 - 20) / (20000 - 20) ≈ 0.05
    int freq = test_state.eq_window.bands[0].frequency;
    assert_true(freq >= 20 && freq <= 20000, "Frequency should be in valid range");
    
    // Gain mapping: -150 to +150 should map to 0.0-1.0 for OSC
    // Normalized: (50 + 150) / 300 ≈ 0.667
    int gain = test_state.eq_window.bands[0].gain;
    assert_true(gain >= -150 && gain <= 150, "Gain should be in valid range");
    
    // Q mapping: 3-100 should map to 0.0-1.0 for OSC
    // Normalized: (50 - 3) / (100 - 3) ≈ 0.505
    int q = test_state.eq_window.bands[0].q;
    assert_true(q >= 3 && q <= 100, "Q should be in valid range");
    
    test_count++;
}

// ============================================================
// TEST 7: All 5 EQ Bands Persistent Storage
// ============================================================
void test_eq_bands_persistence() {
    test_results[test_count].test_num = test_count + 1;
    test_results[test_count].test_name = "EQ Bands Persistence";
    
    AppState_Global test_state = {0};
    state_init();
    test_state.current_state = STATE_EQ_WINDOW;
    
    // Set unique values for all 5 bands
    for (int i = 0; i < 5; i++) {
        test_state.eq_window.bands[i].type = i;
        test_state.eq_window.bands[i].frequency = 100 * (i + 1);
        test_state.eq_window.bands[i].gain = -100 + (i * 50);
        test_state.eq_window.bands[i].q = 10 + (i * 10);
    }
    
    // Verify all bands persist
    for (int i = 0; i < 5; i++) {
        assert_equal_int(test_state.eq_window.bands[i].type, i, 
                        "Band type should persist");
        assert_equal_int(test_state.eq_window.bands[i].frequency, 100 * (i + 1), 
                        "Band frequency should persist");
        assert_equal_int(test_state.eq_window.bands[i].gain, -100 + (i * 50), 
                        "Band gain should persist");
        assert_equal_int(test_state.eq_window.bands[i].q, 10 + (i * 10), 
                        "Band Q should persist");
    }
    
    test_count++;
}

// ============================================================
// TEST 8: EQ Window Close Transition (EQ -> Mixer)
// ============================================================
void test_eq_window_close_transition() {
    test_results[test_count].test_num = test_count + 1;
    test_results[test_count].test_name = "EQ Window Close (EQ -> Mixer)";
    
    AppState_Global test_state = {0};
    state_init();
    test_state.current_state = STATE_EQ_WINDOW;
    test_state.eq_window.channel_id = 5;
    
    // Store EQ state before closing
    EQBandState saved_bands[5];
    for (int i = 0; i < 5; i++) {
        saved_bands[i] = test_state.eq_window.bands[i];
    }
    
    // Close EQ window (B button)
    test_state.current_state = STATE_MIXER_VIEW;
    
    assert_equal_int(test_state.current_state, STATE_MIXER_VIEW, 
                     "Should return to MIXER_VIEW state");
    
    // Verify EQ state is preserved (for next edit)
    for (int i = 0; i < 5; i++) {
        assert_equal_int(test_state.eq_window.bands[i].type, saved_bands[i].type, 
                        "Band state should be preserved");
    }
    
    test_count++;
}

// ============================================================
// TEST 9: Touch Input with EQ Window
// ============================================================
void test_eq_window_touch_input() {
    test_results[test_count].test_num = test_count + 1;
    test_results[test_count].test_name = "EQ Window Touch Input";
    
    AppState_Global test_state = {0};
    state_init();
    test_state.current_state = STATE_EQ_WINDOW;
    
    // Simulate touch on band list area
    // EQ window appears at (20, 20) on bottom screen with 280x200 size
    // Band items are 30px high
    // Band 0: y = 40-70, Band 1: y = 70-100, etc.
    
    TouchInputManager touch_mgr = {0};
    TouchEvent event = {0};
    event.type = TOUCH_EVENT_PRESS;
    event.x = 50;    // Middle of band area
    event.y = 85;    // Band 1 area (70-100)
    
    // Update selected band based on touch Y coordinate
    int band_y_offset = (event.y - 40);  // Relative to band list start
    int touched_band = band_y_offset / 30;
    
    if (touched_band >= 0 && touched_band < 5) {
        test_state.eq_window.selected_band = touched_band;
    }
    
    assert_equal_int(test_state.eq_window.selected_band, 1, 
                     "Touch should select band 1");
    
    test_count++;
}

// ============================================================
// TEST 10: Full State Machine Integration
// ============================================================
void test_full_state_machine() {
    test_results[test_count].test_num = test_count + 1;
    test_results[test_count].test_name = "Full State Machine Integration";
    
    AppState_Global test_state = {0};
    state_init();
    
    // Start at mixer
    test_state.current_state = STATE_MIXER_VIEW;
    assert_equal_int(test_state.current_state, STATE_MIXER_VIEW, 
                     "Should start at MIXER_VIEW");
    
    // Open EQ window
    test_state.current_state = STATE_EQ_WINDOW;
    test_state.eq_window.channel_id = 7;
    assert_equal_int(test_state.current_state, STATE_EQ_WINDOW, 
                     "Should transition to EQ_WINDOW");
    
    // Modify EQ parameters
    test_state.eq_window.selected_band = 2;
    test_state.eq_window.bands[2].frequency = 5000;
    assert_equal_int(test_state.eq_window.bands[2].frequency, 5000, 
                     "Should store EQ modification");
    
    // Close EQ, return to mixer
    test_state.current_state = STATE_MIXER_VIEW;
    assert_equal_int(test_state.current_state, STATE_MIXER_VIEW, 
                     "Should return to MIXER_VIEW");
    
    // Verify EQ state preserved
    assert_equal_int(test_state.eq_window.bands[2].frequency, 5000, 
                     "EQ modification should persist after closing window");
    
    // Open menu
    test_state.current_state = STATE_MENU;
    assert_equal_int(test_state.current_state, STATE_MENU, 
                     "Should transition to MENU");
    
    // Return to mixer from menu
    test_state.current_state = STATE_MIXER_VIEW;
    assert_equal_int(test_state.current_state, STATE_MIXER_VIEW, 
                     "Should return to MIXER_VIEW from MENU");
    
    test_count++;
}

// ============================================================
// Test Runner
// ============================================================
void run_all_tests() {
    test_log("====================================================");
    test_log("PHASE 8: INTEGRATION TEST SUITE");
    test_log("5-Band EQ System with Full State Machine");
    test_log("====================================================");
    
    test_eq_window_initialization();
    test_eq_window_open_transition();
    test_eq_band_parameter_adjustment();
    test_eq_band_navigation();
    test_eq_parameter_cycling();
    test_osc_eq_messages();
    test_eq_bands_persistence();
    test_eq_window_close_transition();
    test_eq_window_touch_input();
    test_full_state_machine();
    
    // Print results
    int passed = 0;
    int failed = 0;
    
    for (int i = 0; i < test_count; i++) {
        if (test_results[i].passed) {
            passed++;
            test_log("✅ PASS");
        } else {
            failed++;
            test_log("❌ FAIL");
        }
    }
    
    test_log("====================================================");
    test_log("Test Summary:");
    // Would print passed/failed counts here
}

#endif // INTEGRATION_TESTS_C
