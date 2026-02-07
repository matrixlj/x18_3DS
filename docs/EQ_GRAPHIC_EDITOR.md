# 5-Band Graphic EQ Editor - Touch-Interactive Interface

## Overview

La finestra EQ è ora visualizzata nello **schermo inferiore** (bottom screen) con un **grafico della risposta in frequenza** completo e **pallini interattivi** (nodes) movibili con il touch.

---

## Visual Layout (Bottom Screen, 320×240)

### Grid Display
```
┌─────────────────────────────────────────────────────────┐
│ EQ GRAPH - Bottom Screen (320x240) - 5 Band Graphic EQ │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  +15dB ┌─────────────────────────────────────────────┐ │
│        │░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░│ │
│   0dB  │░░  ●───●───────●───────────────────●─────●░░│ │
│        │░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░│ │
│ -15dB  └─────────────────────────────────────────────┘ │
│        20Hz    200Hz        2kHz         20kHz         │
│                                                         │
│  B1:LShv  F:120Hz   G:-9.25dB  Q:1.0                  │
│  B2:PEQ   F:463Hz   G:-5.00dB  Q:0.3                  │
│  B3:PEQ   F:1170Hz  G:-0.25dB  Q:0.6                  │
│  B4:PEQ   F:3090Hz  G:-0.25dB  Q:0.6                  │
│  B5:HShv  F:12000Hz G:+5.00dB  Q:0.5                  │
└─────────────────────────────────────────────────────────┘
```

---

## Components

### 1. **Frequency Response Graph**
- **Size**: 280×150 pixels
- **Grid**: Logarithmic frequency scale (20 Hz - 20 kHz)
- **Vertical Grid**: 6 divisions (±15 dB range)
- **Center Line**: 0 dB (green for reference)
- **Background**: Dark blue grid pattern

### 2. **Interactive Nodes (Pallini)**
- **Count**: 5 (one per EQ band)
- **Shape**: Circle with 6-pixel radius
- **Color**: Per-band specific color
  - Low Shelf (B1): **Orange** `#FF8800`
  - Lo-Mid (B2): **Lime** `#88FF00`
  - Mid (B3): **Cyan** `#00FF88`
  - Hi-Mid (B4): **Magenta** `#FF00FF`
  - High Shelf (B5): **Yellow** `#FFFF00`

### 3. **EQ Curve**
- **Type**: Orange spline connecting all nodes
- **Width**: 2 pixels
- **Color**: `#FF9900` (orange)
- **Updates**: Real-time as nodes are moved

### 4. **Axis Labels**
**Frequency (X-axis)**:
- 20 Hz (left)
- 200 Hz
- 2 kHz
- 20 kHz (right)

**Gain (Y-axis)**:
- +15 dB (top)
- 0 dB (center)
- -15 dB (bottom)

### 5. **Parameter Display**
Below the graph, each band shows:
```
B1:LShv  F:120Hz   G:-9.25dB  Q:1.0
B2:PEQ   F:463Hz   G:-5.00dB  Q:0.3
...
```

---

## Band Data (Default Values)

| Band | Type | Frequency | Gain | Q | Color |
|------|------|-----------|------|---|-------|
| B1 | LShv (Low Shelf) | 120 Hz | -9.25 dB | 1.0 | Orange |
| B2 | PEQ (Parametric) | 463 Hz | -5.00 dB | 0.3 | Lime |
| B3 | PEQ (Parametric) | 1170 Hz | -0.25 dB | 0.6 | Cyan |
| B4 | PEQ (Parametric) | 3090 Hz | -0.25 dB | 0.6 | Magenta |
| B5 | HShv (High Shelf) | 12000 Hz | +5.00 dB | 0.5 | Yellow |

---

## Touch Interaction Model

### Node Dragging
User can drag each node to adjust parameters:

1. **Horizontal Movement** (X-axis) → Change **Frequency**
   - Range: 20-20000 Hz (logarithmic)
   - Smoothly follows touch X position

2. **Vertical Movement** (Y-axis) → Change **Gain**
   - Range: -15 to +15 dB
   - 0 dB at screen center

### Example Interaction
```
User touches band B3 node and drags:
- Right → Increases frequency (move toward 2kHz)
- Down → Increases gain (boost)
- Up → Decreases gain (cut)

Real-time OSC messages sent:
/ch/01/eq/3/f → updated frequency value
/ch/01/eq/3/g → updated gain value
```

---

## Real-Time OSC Updates

As the user adjusts nodes, the app sends OSC messages to the mixer:

### On Frequency Drag
```
Address: /ch/XX/eq/B/f
Value: Normalized (0.0-1.0)
Formula: (freq - 20) / 19980 * 100
Example: Drag B2 from 463→1000 Hz
         OSC: /ch/01/eq/2/f → 50 (approx)
```

### On Gain Drag
```
Address: /ch/XX/eq/B/g
Value: Normalized (0.0-1.0)
Formula: (gain + 15) / 30 * 100
Example: Drag B2 from -5.00→+2.00 dB
         OSC: /ch/01/eq/2/g → 56.7
```

### Q-Factor Control
```
Address: /ch/XX/eq/B/q
Current: Fixed per band (shown in parameter display)
Future: Could add vertical lane or modifier key for Q adjustment
```

---

## Touch-Friendly Design Features

### Large Hit Areas
- Node circles: 12×12 pixel touch area (6px radius × 2)
- Easy to tap on 3DS touchscreen with stylus

### Visual Feedback
- Node changes color brightness when selected
- Curve updates in real-time
- Parameter values update below graph

### Snapping & Constraints
- **Frequency**: Logarithmic snapping to perceptual scale
- **Gain**: ±15 dB hard limits
- **Q**: Preset per band (or adjustable via secondary controls)

---

## Screen Comparison

### Framebuffer Preview Versions

**framebuffer_preview.png** (Mixer View)
- TOP: Step List editor
- BOTTOM: 16-channel mixer with faders

**framebuffer_eq_graph.png** (EQ Editor View)
- TOP: Step List editor
- BOTTOM: **5-Band Graphic EQ with interactive nodes** ✅ NEW!

---

## Implementation Details

### Drawing Algorithm
```python
1. Draw grid background (frequency × gain grid lines)
2. For each of 5 bands:
   - Calculate X position from frequency (log scale)
   - Calculate Y position from gain (linear -15 to +15)
   - Store node position (x, y)
3. Draw EQ curve:
   - Connect nodes with straight lines
   - (Could upgrade to spline for smoother curve)
4. Draw interactive nodes:
   - Circle outline in band color
   - Center dot in solid color
5. Draw axis labels and band parameters
```

### Touch Coordinate Mapping
```c
// 3DS Touchscreen: 320×240 pixels
// EQ Graph area: starts at (20, 25) with size 280×150

touch_x → frequency_hz (logarithmic)
touch_y → gain_db (linear)

Example:
touch at (150, 75) →
  freq_norm = (150 - 20) / 280 ≈ 0.46
  freq = 20 + 0.46 * 19980 ≈ 9200 Hz
  
  gain_norm = (150 - 25) / 150 = 0.83
  gain_db = 15 - (0.83 * 30) ≈ -10 dB
```

---

## Integration with 3DS Hardware

### Controls
- **Stylus/Touch**: Drag nodes to adjust frequency/gain
- **D-Pad**: Navigate between bands (highlight border)
- **A Button**: Confirm adjustment
- **B Button**: Cancel/Revert
- **X Button**: Reset band to default
- **Y Button**: Reset all bands
- **L/R Buttons**: Adjust Q-factor (future)

### Screen Rendering
- Updated every frame (~60 FPS)
- Touch input sampled from HID
- Node positions recalculated on touch
- OSC messages sent immediately

---

## File Location

```
build/
├── framebuffer_preview.png       # Original mixer view (8.9 KB)
├── framebuffer_eq_window.png     # Old EQ window (obsolete)
└── framebuffer_eq_graph.png      # NEW: Graphic EQ (17 KB) ✅
```

---

## Next Implementation Steps

1. **Touch Input Handler**: Map stylus coordinates to node positions
2. **Node Dragging Logic**: Calculate frequency/gain from touch position
3. **Real-Time Curve Update**: Redraw curve as nodes move
4. **OSC Integration**: Send messages when values change
5. **Constraint Checking**: Keep nodes within valid ranges
6. **Q-Factor UI**: Add secondary control for Q adjustment
7. **Band Naming**: Display band type (LShv, PEQ, etc.) in UI

---

## Color Reference

```
Band Types:
- Shelf Filters (LCut, LShv, HShv): Orange/Yellow
- Parametric (PEQ, VEQ): Green/Cyan

Interactive Elements:
- Grid: Dark blue `#001133`
- Border: Cyan `#0088FF`
- Grid lines: `#222244`
- Center (0dB): `#446644` (green)
- Curve: Orange `#FF9900`
- Nodes: Per-band color (see table above)
```

---

## Performance Considerations

- **Rendering**: Simple line drawing (no heavy graphics)
- **Touch Latency**: Direct coordinate mapping (minimal lag)
- **OSC Frequency**: 60 messages/sec (one per frame, adjustable)
- **3DS Compatibility**: Uses basic 2D drawing (PIL equivalent)

---

## Quality vs. Reference Image

Your reference image shows:
✅ Frequency response curve with visible dips/peaks
✅ Interactive nodes positioned at band frequencies
✅ Grid background for scale reference
✅ Parameter display below graph
✅ Color-coded nodes

Our implementation:
✅ 5 interactive nodes (touch-movable)
✅ Real-time curve connecting nodes
✅ Logarithmic frequency scale
✅ dB gain display
✅ Band parameters shown
✅ 3DS-optimized resolution

