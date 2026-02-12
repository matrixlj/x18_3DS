#include "common.h"
#include "mixer_window.h"

// Convert fader value (0-1) to dB using calibration points
float fader_value_to_db(float value)
{
    // Calibration points from X18 specs
    // percentages: 0, 13, 26, 39, 51, 63, 76, 88, 100
    // db_values: -100, -50, -30, -20, -10, -5, 0, 5, 10
    
    float percent = value * 100.0f;
    
    // Calibration points
    float calibration_percent[] = {0, 13, 26, 39, 51, 63, 76, 88, 100};
    float calibration_db[] = {-100, -50, -30, -20, -10, -5, 0, 5, 10};
    
    // Find the range this percent falls into and interpolate
    for (int i = 0; i < 8; i++) {
        if (percent >= calibration_percent[i] && percent <= calibration_percent[i+1]) {
            // Linear interpolation between calibration points
            float t = (percent - calibration_percent[i]) / (calibration_percent[i+1] - calibration_percent[i]);
            float db = calibration_db[i] + t * (calibration_db[i+1] - calibration_db[i]);
            return db;
        }
    }
    
    // Default to max if > 100
    return 10.0f;
}
