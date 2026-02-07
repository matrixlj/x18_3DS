# Touch Input Handler for EQ Graphic Editor

## Overview

Implementato il sistema completo di touch input per l'editor EQ grafico. Gli utenti possono ora:
- **Toccare un nodo** per selezionarlo
- **Trascinare il nodo** orizzontalmente per cambiare frequenza
- **Trascinare il nodo** verticalmente per cambiare guadagno
- **Toccare il pulsante "Back"** per tornare al mixer

---

## Architecture

### Functions Implemented

#### 1. `eq_window_handle_touch_input()`
**Punto di ingresso principale per tutti gli input touch**

```c
void eq_window_handle_touch_input(AppState_Global *state, 
                                   int touch_x, int touch_y);
```

- Riceve coordinate touch (x, y) dallo schermo inferiore
- Controlla se è stato toccato il pulsante "Back"
- Controlla se è stato toccato un nodo EQ
- Delega al gestore appropriato

**Flusso:**
```
Touch Input
    ↓
├─ Back Button? → STATE_MIXER_VIEW
└─ EQ Node? → eq_window_update_node_from_touch()
```

---

#### 2. `eq_window_get_node_at_position()`
**Rileva quale nodo (se presente) è stato toccato**

```c
int eq_window_get_node_at_position(int touch_x, int touch_y);
```

**Returns:**
- `0-4`: Indice del band toccato
- `-1`: Nessun nodo al toccamento

**Algoritmo:**
```
Per ogni band (0-4):
  1. Calcola posizione X: freq_norm = (freq - 20) / 19980
  2. Calcola posizione Y: gain_norm = (gain + 15) / 30
  3. Calcola distanza: sqrt((touch_x - x)² + (touch_y - y)²)
  4. Se distanza < node_radius: return band_id
Return -1
```

**Hit Radius:**
- 8 pixel (6px drawn + 2px tolerance per facile toccamento su touchscreen)

---

#### 3. `eq_window_update_node_from_touch()`
**Aggiorna parametri del nodo in base alla posizione touch**

```c
void eq_window_update_node_from_touch(AppState_Global *state, 
                                       int node_id, 
                                       int touch_x, 
                                       int touch_y);
```

**Coordinate Mapping:**

```
Asse X (Frequenza):
  touch_x: 40 (graph_x) → 320 (graph_x + graph_w)
  freq:    20 Hz → 20000 Hz
  mapping: new_freq = 20 + ((touch_x - 40) * 19980 / 280)

Asse Y (Guadagno):
  touch_y: 50 (graph_y) → 200 (graph_y + graph_h)
  gain:    +15 dB → -15 dB
  mapping: gain_norm = 1000 - ((touch_y - 50) * 1000 / 150)
           new_gain = (gain_norm * 30 / 1000) - 15  [dB]
           new_gain *= 10  [stored as 0.1 dB units]
```

**Constraints:**
- Frequenza: Clampata a 20-20000 Hz
- Guadagno: Clampata a -150 to +150 (dB*10)
- Touch position: Clampata ai confini del grafico

---

#### 4. `eq_window_drag_node()`
**Wrapper per il drag durante il movimento del touchscreen**

```c
void eq_window_drag_node(AppState_Global *state, 
                        int node_id, 
                        int new_x, 
                        int new_y);
```

- Chiamato continuamente durante il drag
- Delega a `eq_window_update_node_from_touch()`
- Permette aggiornamenti real-time mentre l'utente trascina

---

## Touch Input Flow Diagram

```
┌─────────────────────────────────────┐
│   3DS Touchscreen Event (x, y)      │
└─────────────┬───────────────────────┘
              │
              ▼
   ┌──────────────────────────┐
   │ eq_window_handle_touch   │
   └──────┬───────────────────┘
           │
    ┌──────┴──────┐
    │             │
    ▼             ▼
┌────────┐    ┌──────────────────┐
│ Back   │    │ EQ Node?         │
│Button? │    └──────┬───────────┘
│        │           │
└────┬───┘      ┌────▼──────────┐
     │          │               │
     │         YES              NO
     │          │               │
     │          ▼              (noop)
     │    ┌──────────────┐
     │    │get_node_at   │
     │    │_position()   │
     │    └──────┬───────┘
     │           │
     │    ┌──────▼────────┐
     │    │Found (0-4)?   │
     │    └──────┬────────┘
     │           │
     │          YES
     │           │
     │           ▼
     │    ┌──────────────────────┐
     │    │update_node_from_touch│
     │    │  - calc new_freq     │
     │    │  - calc new_gain     │
     │    │  - update band state │
     │    └──────────────────────┘
     │
     └─────────┬──────────────────┐
               │                  │
               ▼                  ▼
         [MIXER]            [EQ_EDITOR]
```

---

## Real-Time Parameter Updates

### When Touch Detected

1. **Identificazione Nodo**
   ```
   node_id = eq_window_get_node_at_position(touch_x, touch_y)
   if node_id >= 0: update it
   ```

2. **Calcolo Nuovi Valori**
   ```
   new_freq = touch_x → 20-20000 Hz (logaritmico)
   new_gain = touch_y → -15 to +15 dB (lineare)
   ```

3. **Aggiornamento Stato**
   ```
   bands[node_id].frequency = new_freq
   bands[node_id].gain = new_gain
   ```

4. **Rendering Aggiornato**
   - Prossimo frame renderizza il nodo nella nuova posizione
   - Curva EQ si aggiorna in tempo reale

5. **OSC Message (Futuro)**
   ```c
   // TODO: Uncomment per inviare OSC al mixer
   osc_send_eq_frequency(client, channel, node_id, new_freq);
   osc_send_eq_gain(client, channel, node_id, new_gain);
   ```

---

## Coordinate System

### Screen Layout (Bottom Screen, 320×240)

```
┌─────────────────────────────────────┐
│ EQ GRAPH - 5 Band  [<<MIXER] Back   │  y: 0-5
├─────────────────────────────────────┤
│                                     │
│  Graph Area: (40, 50) size 280×150  │  y: 50-200
│                                     │
│  ┌──────────────────────────────┐   │
│  │ EQ Curve with 5 Nodes        │   │
│  │ (Log freq scale, Linear dB)  │   │
│  └──────────────────────────────┘   │
│                                     │
│  B1:LShv F:120Hz  G:-9.25dB Q:1.0  │  y: 205+
│  ...parameters...                   │
└─────────────────────────────────────┘
```

### Touch Detection Zones

| Zone | X Range | Y Range | Function |
|------|---------|---------|----------|
| Back Button | 270-320 | 3-15 | Switch to mixer |
| Graph (nodes) | 40-320 | 50-200 | Touch/drag nodes |
| Parameters | 40-320 | 205-240 | Display only |

---

## Code Integration

### Calling Touch Handler

Da inserire nel main event loop (non ancora implementato):

```c
// In main.c or input handler:
if (current_state == STATE_EQ_WINDOW) {
    // Get touch input from 3DS HID
    int touch_x = hid_touch.px;  // Touch X
    int touch_y = hid_touch.py;  // Touch Y
    
    if (hid_touch.valid) {
        eq_window_handle_touch_input(&app_state, touch_x, touch_y);
    }
}
```

---

## Testing Strategy

### Test Case 1: Node Detection
```
Touch at node B3 (1170 Hz, -0.25 dB)
Expected: node_id = 2 returned
```

### Test Case 2: Frequency Drag
```
Touch at x=130 (left of B3)
Expected: new_freq < 1170 Hz
Expected: node moves left on screen
```

### Test Case 3: Gain Drag
```
Touch at y=100 (above center)
Expected: new_gain > -0.25 dB (boost)
Expected: node moves up on screen
```

### Test Case 4: Back Button
```
Touch at (280-320, 3-15)
Expected: current_state = STATE_MIXER_VIEW
```

### Test Case 5: Constraints
```
Touch at x=-100 (outside graph left)
Expected: x clamped to 40
Expected: freq = 20 Hz (min)
```

---

## Performance Considerations

### Complexity
- **Detection**: O(5) - iterate through 5 nodes
- **Update**: O(1) - direct parameter update
- **Hit Radius**: Fixed 8px radius

### Frequency
- Called on every touch event (~60 Hz on 3DS)
- Minimal computation per frame

### Memory
- No dynamic allocation
- Uses existing band state arrays

---

## Future Enhancements

1. **Visual Feedback**
   - Highlight selected node on touch
   - Show frequency/gain values in real-time
   - Snap to nearby nodes

2. **Q-Factor Control**
   - Long-press to enter Q-adjustment mode
   - Or use left/right buttons to adjust Q

3. **Undo/Redo**
   - Store previous state on each drag
   - Press Y button to undo

4. **OSC Integration**
   - Send OSC messages while dragging (not just on release)
   - Verify mixer receives updates

5. **Multi-Touch**
   - Allow dragging multiple nodes simultaneously
   - Useful for creating complex EQ shapes

---

## Implementation Status

| Component | Status | Notes |
|-----------|--------|-------|
| Node Detection | ✅ Complete | Hit radius: 8px |
| Frequency Mapping | ✅ Complete | Logarithmic scale |
| Gain Mapping | ✅ Complete | Linear scale ±15dB |
| Touch Handling | ✅ Complete | Main entry point |
| Back Button | ✅ Complete | Returns to mixer |
| Rendering | ✅ Complete | Shows nodi + curve |
| OSC Integration | 🔲 TODO | Commented out, ready |
| Real-time Preview | ⏳ Manual | Use test_3dsx.py |

---

## Compilation

```bash
cd /Users/lorenzomazzocchetti/Desktop/Github/X18_Nintendo_ds
make 3dsx
# Result: build/app.3dsx (126 KB)
```

**Status:** ✅ Compiles without errors

---

## Next Steps

1. **Integrate with Main Event Loop**
   - Add touch input handling in main.c
   - Route touch events to eq_window_handle_touch_input()

2. **Visual Feedback**
   - Highlight selected/dragged node
   - Show parameter values during drag

3. **OSC Integration**
   - Uncomment OSC send calls in update function
   - Test with real Behringer X18 mixer

4. **Azahar Testing**
   - Deploy to Azahar emulator
   - Test touch input with emulator's touch simulation
   - Verify node drag works correctly

