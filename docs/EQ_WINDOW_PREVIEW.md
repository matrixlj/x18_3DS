# EQ Window Framebuffer Preview

## Overview

Sono state generate **due preview visuali** del framebuffer 3DS che mostrano l'interfaccia della finestra EQ:

### 1. **framebuffer_preview.png** (8.9 KB)
- TOP screen: Step List editor
- BOTTOM screen: 16-channel mixer

### 2. **framebuffer_eq_window.png** (19 KB) ← **EQ WINDOW VERSION**
- TOP screen: **5-Band Parametric EQ Editor** (NEW!)
- BOTTOM screen: 16-channel mixer (shared)

---

## EQ Window Layout (Top Screen, 400×240)

### Header Section
```
═══════════════════════════════════════════════════════════
  EQ WINDOW - Top Screen (400x240)
═══════════════════════════════════════════════════════════
  5-Band EQ - Channel 01
```

### Band Display Structure
Each of the 5 EQ bands is displayed as a compact row:

```
┌─────────────────────────────────────────────────────┐
│ B1:LCut  120Hz      Gain:  0.0dB Q:0.3              │
│ [====|─────────────────────────────────────────] │
│                                                     │
│ B2:LShv  500Hz      Gain: +3.0dB Q:2.0              │
│ [────────|──────────────────────────────────────] │
│                                                     │
│ B3:PEQ   2000Hz     Gain: +6.0dB Q:5.0              │
│ [───────────────────|────────────────────────────] │
│                                                     │
│ B4:VEQ   8000Hz     Gain: -3.0dB Q:3.0              │
│ [──────────────────────────|──────────────────────] │
│                                                     │
│ B5:HShv  12000Hz    Gain:  0.0dB Q:0.3              │
│ [────────────────────────────|────────────────────] │
└─────────────────────────────────────────────────────┘
```

### Elements per Band:
- **Band Label**: B1-B5 (identifies the band number)
- **Filter Type**: LCut, LShv, PEQ, VEQ, HShv (color-coded)
- **Frequency**: Current frequency in Hz (20-20000)
- **Gain**: Current gain in dB (-15.0 to +15.0)
- **Q-Factor**: Current Q value (0.3-10.0)
- **Frequency Slider**: Visual representation of frequency position

### Color Coding:
- **Shelf Filters** (LCut, LShv, HShv): `#FF8800` (Orange)
- **Parametric Filters** (PEQ, VEQ): `#88FF00` (Lime Green)
- **Background**: `#332200` (Dark Brown/Gold)

---

## Mixer Screen (Bottom, 320×240)

The bottom screen shows the **16-channel mixer** simultaneously:

```
[MIXER - Bottom Screen (320x240)        ]
[Step: 1 | IP: 192.168.1.100           ]
[  01  02  03  04  05  06  07  08      ]
[  ▓▓  ▓▓  ▓▓  ▓▓  ▓▓  ▓▓  ▓▓  ▓▓      ]
[  ▓▓  ▓▓  ▓▓  ▓▓  ▓▓  ▓▓  ▓▓  ▓▓      ]
[  ║   ║   ║   ║   ║   ║   ║   ║  ▐──  ]
[  ║   ║   ║   ║   ║   ║   ║   ║  ▐──  ]
[  [M] [M] [M] [M] [M] [M] [M] [M]      ]
[                                       ]
[  09  10  11  12  13  14  15  16      ]
[  ▓▓  ▓▓  ▓▓  ▓▓  ▓▓  ▓▓  ▓▓  ▓▓      ]
[  ▓▓  ▓▓  ▓▓  ▓▓  ▓▓  ▓▓  ▓▓  ▓▓      ]
[  ║   ║   ║   ║   ║   ║   ║   ║  ▐──  ]
[  ║   ║   ║   ║   ║   ║   ║   ║  ▐──  ]
[  [M] [M] [M] [M] [M] [M] [M] [M]      ]
[Ch: 01 | Mute: OFF | EQ: 5 bands      ]
```

- **Top Row**: Channels 01-08
- **Bottom Row**: Channels 09-16
- **Fader Position**: 75% on Ch01, 50% on others
- **Mute Buttons**: Red indicator per channel

---

## Interaction Flow

### Scenario: Edit Channel 1 EQ Band 3 (PEQ at 2 kHz)

1. **User presses EQ button** → TOP screen switches to EQ window
2. **Band 3 selected** → Shows:
   - Type: PEQ (Parametric EQ)
   - Frequency: 2000 Hz
   - Gain: +6.0 dB
   - Q: 5.0

3. **User adjusts parameters**:
   - D-Pad UP: Increase frequency
   - D-Pad DOWN: Decrease frequency
   - R + D-Pad: Adjust gain
   - L + D-Pad: Adjust Q-factor

4. **Real-time OSC messages sent**:
   ```
   /ch/01/eq/3/type → 2 (PEQ)
   /ch/01/eq/3/f → 51 (normalized freq)
   /ch/01/eq/3/g → 70 (normalized gain)
   /ch/01/eq/3/q → ~49 (normalized Q)
   ```

5. **Changes reflected on mixer** → Bottom screen shows updated EQ status

---

## Visual Features

### Dual-Screen Display
- **420 × 510 pixel** PNG image
- TOP: 400 × 240 (EQ Window)
- BOTTOM: 320 × 240 (Mixer)
- 30px vertical spacing
- 10px side borders

### Color Scheme
- **Background**: Dark gray `#222222`
- **EQ Window**: Dark brown frame `#332200`, gold border `#FFAA00`
- **Shelf Filters**: Orange `#FF8800`
- **Parametric**: Lime green `#88FF00`
- **Mixer**: Red border `#FF0000`, dark background `#110000`
- **Active Channel**: Cyan `#00FFFF`
- **Text**: Yellow/White for visibility

### Font Information
- Monospace text rendering
- Channel numbers: 2-character zero-padded (01-16)
- Parameter values: Full precision for dB and Hz

---

## Technical Implementation

### Rendering Functions (test_3dsx.py)
```python
def create_test_framebuffer(show_eq_window=False):
    # Generate TOP screen based on show_eq_window flag
    if show_eq_window:
        # Render 5-band EQ window with sliders
    else:
        # Render step list editor
    
    # Always render BOTTOM screen (mixer)
    # Combine into 420x510 PNG
```

### Parameter Display
- **Frequency**: Hz (20-20000)
- **Gain**: dB with sign (+6.0, -3.0, 0.0)
- **Q-Factor**: Decimal (0.3-10.0)
- **Sliders**: Normalized position based on parameter value

---

## File Locations

```
build/
├── app.3dsx                      # Main executable (126 KB)
├── framebuffer_preview.png       # Step List view (8.9 KB)
└── framebuffer_eq_window.png     # EQ Window view (19 KB) ← NEW!
```

---

## Next Steps

1. **Hook EQ Window to Actual State**: Link rendering to real `AppState_Global` data
2. **Input Handling**: Implement D-Pad/Button controls for parameter adjustment
3. **OSC Integration**: Send actual OSC messages when parameters change
4. **Real 3DS Testing**: Deploy to actual hardware with 3DS Homebrew Launcher
5. **Network Testing**: Connect to real Behringer X18 mixer over network

---

## File Generation Command

To regenerate the preview images:
```bash
python3 test_3dsx.py build/app.3dsx
```

This automatically generates both:
- `framebuffer_preview.png` (Step List mode)
- `framebuffer_eq_window.png` (EQ Window mode)

Both show the complete dual-screen interface with realistic 3DS dimensions.
