# OSC Test Samples for X18 Mixer 3DS

This file contains example OSC commands you can use to test the emulator.

## Quick Test Commands

### Using nc (netcat) on macOS

```bash
# Test connection - query mixer info
echo -ne "/info\x00\x00\x00" | nc 127.0.0.1 10024

# The response should be the mixer info
```

### Using X32_Command tool (from X32-Behringer)

From another terminal, you can use the X32_Command tool for easier testing:

```bash
cd /path/to/X32-Behringer/build

# Interactive mode
./X32_Command -i 127.0.0.1

# Then type commands like:
# /ch/01/mix/fader ,f 0.5
# /info
# exit
```

---

## Test Sequence: Channel Fader

**Objective**: Test sending and receiving channel fader values

### 1. Query Current Fader Value
```
Command: /ch/01/mix/fader
Expected Response: /ch/01/mix/fader ,f [current_value]
```

### 2. Set Fader to 50% (0dB equivalent)
```
Command: /ch/01/mix/fader ,f 0.76
```

### 3. Set Fader to Minimum (-∞)
```
Command: /ch/01/mix/fader ,f 0.0
```

### 4. Set Fader to Maximum (+10dB)
```
Command: /ch/01/mix/fader ,f 1.0
```

### 5. Verify with Query
```
Command: /ch/01/mix/fader
Expected: /ch/01/mix/fader ,f [0.0 | 0.76 | 1.0]
```

---

## Test Sequence: Channel Mute

### 1. Query Mute Status
```
Command: /ch/01/mix/on
Expected Response: /ch/01/mix/on ,i [0 | 1]
```

### 2. Mute Channel (0 = muted in X32)
```
Command: /ch/01/mix/on ,i 0
```

### 3. Unmute Channel (1 = unmuted)
```
Command: /ch/01/mix/on ,i 1
```

---

## Test Sequence: EQ - Band 1 (LCut)

**Objective**: Test EQ control, starting with Band 1 (should be LCut by default)

### 1. Query Band 1 Type
```
Command: /ch/01/eq/1/type
Expected Response: /ch/01/eq/1/type ,i 0  (0 = LCut)
```

### 2. Query Frequency
```
Command: /ch/01/eq/1/f
Expected Response: /ch/01/eq/1/f ,f [value 0.0-1.0]
```

### 3. Set Frequency to 200Hz (normalized ~0.3)
```
Command: /ch/01/eq/1/f ,f 0.3
```

### 4. Query Gain
```
Command: /ch/01/eq/1/g
Expected Response: /ch/01/eq/1/g ,f [value]
```

### 5. Set Gain (cutting bass - negative gain)
```
Command: /ch/01/eq/1/g ,f -0.5  # -50% gain = cutting
```

---

## Test Sequence: EQ - Band 2 (PEQ)

### 1. Verify Band 2 Type
```
Command: /ch/01/eq/2/type
Expected Response: /ch/01/eq/2/type ,i 2  (2 = PEQ)
```

### 2. Set Frequency to 1kHz
```
Command: /ch/01/eq/2/f ,f 0.5  # Approximate normalized value
```

### 3. Set Positive Gain (boosting)
```
Command: /ch/01/eq/2/g ,f 0.5  # +50% gain
```

### 4. Set Q (width of peak)
```
Command: /ch/01/eq/2/q ,f 0.7
```

---

## Test Sequence: Multi-Channel

### Fade Multiple Channels
```
/ch/01/mix/fader ,f 0.5
/ch/02/mix/fader ,f 0.6
/ch/03/mix/fader ,f 0.7
/ch/04/mix/fader ,f 0.8
/ch/05/mix/fader ,f 0.9
```

### Query All at Once
```
/ch/01/mix/fader
/ch/02/mix/fader
/ch/03/mix/fader
/ch/04/mix/fader
/ch/05/mix/fader
```

---

## Test Sequence: Batch Operation

### Set Complete Mix Setup
```
# Mute all channels except 1-4
/ch/05/mix/on ,i 0
/ch/06/mix/on ,i 0
/ch/07/mix/on ,i 0
/ch/08/mix/on ,i 0
/ch/09/mix/on ,i 0
/ch/10/mix/on ,i 0
/ch/11/mix/on ,i 0
/ch/12/mix/on ,i 0
/ch/13/mix/on ,i 0
/ch/14/mix/on ,i 0
/ch/15/mix/on ,i 0
/ch/16/mix/on ,i 0

# Set faders for active channels
/ch/01/mix/fader ,f 0.8
/ch/02/mix/fader ,f 0.7
/ch/03/mix/fader ,f 0.6
/ch/04/mix/fader ,f 0.5

# Apply EQ to channel 1
/ch/01/eq/1/type ,i 0  # LCut
/ch/01/eq/1/f ,f 0.2
/ch/01/eq/1/g ,f -0.3

/ch/01/eq/2/type ,i 2  # PEQ
/ch/01/eq/2/f ,f 0.5
/ch/01/eq/2/g ,f 0.4
/ch/01/eq/2/q ,f 0.7
```

---

## Binary Encoding for Raw OSC

If sending raw binary OSC (using `nc` or sockets):

### Example: `/ch/01/mix/fader ,f 0.5`

```
Hex: 2f 63 68 2f 30 31 2f 6d 69 78 2f 66 61 64 65 72 
     00 00 00 00 2c 66 00 00 3f 00 00 00

Breaking down:
2f 63 68 2f 30 31 2f 6d 69 78 2f 66 61 64 65 72 00 00 00 00
= "/ch/01/mix/fader" + 4 padding nulls (to align to 4-byte boundary)

2c 66 00 00
= ",f" + 2 padding nulls

3f 00 00 00
= 0.5 in IEEE 754 float format
```

**Pro tip**: Use X32_Command or proper OSC library - raw binary encoding is error-prone.

---

## Emulator Response Format

The emulator will:

1. **Echo back** any command it receives (prefixed with "->X,  XX B:")
2. **Respond** with the value (prefixed with "X->,  XX B:")

Example output in verbose mode:
```
->X,   28 B: /ch/01/mix/fader~~~~,f~~[0.5000]
X->,   28 B: /ch/01/mix/fader~~~~,f~~[0.5000]
```

(The `~~~~` symbols are null bytes for padding)

---

## Notes

- **Channels**: 01-16 (use zero-padding: `/ch/01`, not `/ch/1`)
- **Value Range**: Float values typically 0.0 - 1.0 (normalized)
- **Frequency**: For EQ, frequencies are normalized to 0.0-1.0 range (maps to Hz internally)
- **Gain**: Positive = boosting, Negative = cutting
- **Q Factor**: Higher = narrower peak (for PEQ/VPEQ)
- **Type Values**: 0=LCut, 1=LShv, 2=PEQ, 3=VPEQ, 4=HShv, 5=HCut

---

## Debugging OSC Issues

### Check Emulator Logs
```bash
tail -f x32-emulator.log
```

### Filter for Specific Channel
```bash
tail -f x32-emulator.log | grep "ch/05"
```

### Count Commands Received
```bash
grep "->X" x32-emulator.log | wc -l
```

---

**File Created**: 9 febbraio 2026  
**Reference**: X32 OSC Protocol @ https://sites.google.com/site/patrickmaillot/x32
