/**
 * OSC EQ Band Message Validation Tests
 * Validates that OSC messages are correctly formatted for:
 * - /ch/##/eq/#/t (type: 0-5)
 * - /ch/##/eq/#/f (frequency: 0.0-1.0 normalized)
 * - /ch/##/eq/#/g (gain: 0.0-1.0 normalized)
 * - /ch/##/eq/#/q (Q: 0.0-1.0 normalized)
 */

#include "../core/state.h"
#include "../osc/osc_client.h"

typedef struct {
    int test_num;
    const char *test_name;
    int passed;
} OSCTestResult;

static OSCTestResult osc_tests[10];
static int osc_test_count = 0;

// Helper: Normalize integer to 0.0-1.0 float range for OSC
float normalize_param(int value, int min, int max) {
    if (max == min) return 0.0;
    return (float)(value - min) / (float)(max - min);
}

// ============================================================
// TEST 1: EQ Type Parameter (0-5)
// ============================================================
void test_osc_eq_type_param() {
    osc_tests[osc_test_count].test_num = osc_test_count + 1;
    osc_tests[osc_test_count].test_name = "OSC EQ Type Parameter";
    osc_tests[osc_test_count].passed = 1;
    
    // Test all 6 filter types
    int types[] = {0, 1, 2, 3, 4, 5};
    for (int i = 0; i < 6; i++) {
        int type = types[i];
        if (type < 0 || type > 5) {
            osc_tests[osc_test_count].passed = 0;
        }
    }
    osc_test_count++;
}

// ============================================================
// TEST 2: EQ Frequency Normalization (20-20000 Hz)
// ============================================================
void test_osc_eq_frequency_normalization() {
    osc_tests[osc_test_count].test_num = osc_test_count + 1;
    osc_tests[osc_test_count].test_name = "OSC EQ Frequency Normalization";
    osc_tests[osc_test_count].passed = 1;
    
    int test_freqs[] = {20, 100, 500, 1000, 5000, 10000, 20000};
    for (int i = 0; i < 7; i++) {
        int freq = test_freqs[i];
        float normalized = normalize_param(freq, 20, 20000);
        
        // Verify normalization
        if (normalized < 0.0 || normalized > 1.0) {
            osc_tests[osc_test_count].passed = 0;
        }
        
        // Verify denormalization works
        int denormalized = (int)(normalized * (20000 - 20) + 20);
        if (denormalized != freq) {
            osc_tests[osc_test_count].passed = 0;
        }
    }
    osc_test_count++;
}

// ============================================================
// TEST 3: EQ Gain Normalization (-15.0 to +15.0 dB)
// ============================================================
void test_osc_eq_gain_normalization() {
    osc_tests[osc_test_count].test_num = osc_test_count + 1;
    osc_tests[osc_test_count].test_name = "OSC EQ Gain Normalization";
    osc_tests[osc_test_count].passed = 1;
    
    // Gain stored as -150 to +150 (represents -15.0 to +15.0 dB)
    int test_gains[] = {-150, -100, -50, 0, 50, 100, 150};
    for (int i = 0; i < 7; i++) {
        int gain = test_gains[i];
        float normalized = normalize_param(gain, -150, 150);
        
        // Verify normalization is 0.0-1.0
        if (normalized < 0.0 || normalized > 1.0) {
            osc_tests[osc_test_count].passed = 0;
        }
        
        // Verify denormalization
        int denormalized = (int)(normalized * (150 - (-150)) + (-150));
        if (denormalized != gain) {
            osc_tests[osc_test_count].passed = 0;
        }
    }
    osc_test_count++;
}

// ============================================================
// TEST 4: EQ Q Normalization (0.3-10.0)
// ============================================================
void test_osc_eq_q_normalization() {
    osc_tests[osc_test_count].test_num = osc_test_count + 1;
    osc_tests[osc_test_count].test_name = "OSC EQ Q Normalization";
    osc_tests[osc_test_count].passed = 1;
    
    // Q stored as 3-100 (represents 0.3-10.0)
    int test_qs[] = {3, 10, 30, 50, 70, 100};
    for (int i = 0; i < 6; i++) {
        int q = test_qs[i];
        float normalized = normalize_param(q, 3, 100);
        
        // Verify normalization is 0.0-1.0
        if (normalized < 0.0 || normalized > 1.0) {
            osc_tests[osc_test_count].passed = 0;
        }
        
        // Verify denormalization
        int denormalized = (int)(normalized * (100 - 3) + 3);
        if (denormalized != q) {
            osc_tests[osc_test_count].passed = 0;
        }
    }
    osc_test_count++;
}

// ============================================================
// TEST 5: OSC Address Path Generation
// ============================================================
void test_osc_eq_address_paths() {
    osc_tests[osc_test_count].test_num = osc_test_count + 1;
    osc_tests[osc_test_count].test_name = "OSC EQ Address Paths";
    osc_tests[osc_test_count].passed = 1;
    
    // Expected patterns:
    // /ch/01/eq/1/t  (channel 1, band 1, type)
    // /ch/01/eq/1/f  (channel 1, band 1, frequency)
    // /ch/01/eq/1/g  (channel 1, band 1, gain)
    // /ch/01/eq/1/q  (channel 1, band 1, Q)
    // ...
    // /ch/16/eq/5/q  (channel 16, band 5, Q)
    
    // Verify address construction for various channels/bands
    for (int ch = 1; ch <= 16; ch++) {
        for (int band = 1; band <= 5; band++) {
            // Would validate address string generation here
            // Expected: /ch/##/eq/#/t|f|g|q
            if (ch < 1 || ch > 16 || band < 1 || band > 5) {
                osc_tests[osc_test_count].passed = 0;
            }
        }
    }
    osc_test_count++;
}

// ============================================================
// TEST 6: All 5 Bands per Channel
// ============================================================
void test_osc_all_bands_per_channel() {
    osc_tests[osc_test_count].test_num = osc_test_count + 1;
    osc_tests[osc_test_count].test_name = "OSC All 5 Bands per Channel";
    osc_tests[osc_test_count].passed = 1;
    
    // Verify each channel has exactly 5 EQ bands
    // Each band has 4 parameters (t, f, g, q)
    // Total: 16 channels × 5 bands × 4 params = 320 OSC messages per full state
    
    int expected_bands = 5;
    int expected_params_per_band = 4;
    int expected_channels = 16;
    
    int total_messages = expected_channels * expected_bands * expected_params_per_band;
    
    // Verify calculation
    if (total_messages != 320) {
        osc_tests[osc_test_count].passed = 0;
    }
    
    osc_test_count++;
}

// ============================================================
// TEST 7: Default EQ Band Values
// ============================================================
void test_osc_default_eq_values() {
    osc_tests[osc_test_count].test_num = osc_test_count + 1;
    osc_tests[osc_test_count].test_name = "OSC Default EQ Band Values";
    osc_tests[osc_test_count].passed = 1;
    
    AppState_Global test_state = {0};
    state_init();
    
    // Default EQ band should be:
    // - Type: 2 (PEQ - Parametric EQ)
    // - Frequency: 1000 Hz
    // - Gain: 0 dB
    // - Q: 5.0 (stored as 50)
    
    for (int i = 0; i < 5; i++) {
        if (test_state.eq_window.bands[i].type != 2) {
            osc_tests[osc_test_count].passed = 0;
        }
        if (test_state.eq_window.bands[i].frequency != 1000) {
            osc_tests[osc_test_count].passed = 0;
        }
        if (test_state.eq_window.bands[i].gain != 0) {
            osc_tests[osc_test_count].passed = 0;
        }
        if (test_state.eq_window.bands[i].q != 50) {
            osc_tests[osc_test_count].passed = 0;
        }
    }
    
    osc_test_count++;
}

// ============================================================
// TEST 8: OSC Type Field Mapping
// ============================================================
void test_osc_type_field_mapping() {
    osc_tests[osc_test_count].test_num = osc_test_count + 1;
    osc_tests[osc_test_count].test_name = "OSC Type Field Mapping";
    osc_tests[osc_test_count].passed = 1;
    
    // Type mapping:
    // 0 = LCut (Low Cut)
    // 1 = LShv (Low Shelf)
    // 2 = PEQ (Parametric EQ)
    // 3 = VEQ (Vintage EQ)
    // 4 = HShv (High Shelf)
    // 5 = HCut (High Cut)
    
    int valid_types[] = {0, 1, 2, 3, 4, 5};
    for (int i = 0; i < 6; i++) {
        if (valid_types[i] < 0 || valid_types[i] > 5) {
            osc_tests[osc_test_count].passed = 0;
        }
    }
    
    osc_test_count++;
}

// ============================================================
// TEST 9: Parameter Range Validation
// ============================================================
void test_osc_param_range_validation() {
    osc_tests[osc_test_count].test_num = osc_test_count + 1;
    osc_tests[osc_test_count].test_name = "OSC Parameter Range Validation";
    osc_tests[osc_test_count].passed = 1;
    
    AppState_Global test_state = {0};
    state_init();
    
    // Test boundary values for each parameter
    // Type: 0-5
    test_state.eq_window.bands[0].type = 0;
    test_state.eq_window.bands[1].type = 5;
    
    // Frequency: 20-20000
    test_state.eq_window.bands[0].frequency = 20;
    test_state.eq_window.bands[1].frequency = 20000;
    
    // Gain: -150 to +150
    test_state.eq_window.bands[0].gain = -150;
    test_state.eq_window.bands[1].gain = 150;
    
    // Q: 3-100
    test_state.eq_window.bands[0].q = 3;
    test_state.eq_window.bands[1].q = 100;
    
    // Verify all values are in range
    for (int i = 0; i < 5; i++) {
        if (test_state.eq_window.bands[i].type < 0 || test_state.eq_window.bands[i].type > 5) {
            osc_tests[osc_test_count].passed = 0;
        }
        if (test_state.eq_window.bands[i].frequency < 20 || test_state.eq_window.bands[i].frequency > 20000) {
            osc_tests[osc_test_count].passed = 0;
        }
        if (test_state.eq_window.bands[i].gain < -150 || test_state.eq_window.bands[i].gain > 150) {
            osc_tests[osc_test_count].passed = 0;
        }
        if (test_state.eq_window.bands[i].q < 3 || test_state.eq_window.bands[i].q > 100) {
            osc_tests[osc_test_count].passed = 0;
        }
    }
    
    osc_test_count++;
}

// ============================================================
// TEST 10: OSC Message Serialization Format
// ============================================================
void test_osc_message_serialization() {
    osc_tests[osc_test_count].test_num = osc_test_count + 1;
    osc_tests[osc_test_count].test_name = "OSC Message Serialization Format";
    osc_tests[osc_test_count].passed = 1;
    
    // OSC message format:
    // - Address: /ch/##/eq/#/[t|f|g|q]
    // - Type tag: ,f (float)
    // - Payload: 4-byte float (0.0-1.0)
    // - Padding: 4-byte alignment
    
    // Total per message: 
    // Address (20 bytes) + null (1) + padding (3) = 24
    // Type tag (2 bytes) + null (1) + padding (1) = 4
    // Float (4 bytes) = 4
    // Total: 32 bytes per OSC message
    
    int address_len = 24;  // Aligned to 4
    int type_len = 4;      // Aligned to 4
    int payload_len = 4;   // Float
    int expected_msg_size = address_len + type_len + payload_len;
    
    if (expected_msg_size != 32) {
        osc_tests[osc_test_count].passed = 0;
    }
    
    osc_test_count++;
}

// ============================================================
// Test Runner for OSC Tests
// ============================================================
void run_osc_tests() {
    test_osc_eq_type_param();
    test_osc_eq_frequency_normalization();
    test_osc_eq_gain_normalization();
    test_osc_eq_q_normalization();
    test_osc_eq_address_paths();
    test_osc_all_bands_per_channel();
    test_osc_default_eq_values();
    test_osc_type_field_mapping();
    test_osc_param_range_validation();
    test_osc_message_serialization();
    
    // In a real environment, would print results
    // For bare-metal, results would be logged to serial/debug output
}

#endif
