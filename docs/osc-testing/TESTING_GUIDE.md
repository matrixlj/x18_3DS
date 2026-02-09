# X18 Mixer 3DS - OSC Testing Guide

## Overview

This directory contains tools and documentation for testing **OSC (Open Sound Control)** communication with a Behringer X32/X18 style mixer.

### Why X32 Emulator?

The X32 emulator serves as a perfect testing tool for X18 OSC because:
- **Compatible Protocol**: X32 and X18 use the same OSC message format (at least for the features we care about: faders, mute, EQ)
- **No Hardware Required**: Test OSC without a physical mixer
- **Deterministic Behavior**: Consistent responses for reproducible testing
- **Development Friendly**: Easy to start/stop, logs all commands

---

## Quick Start

### 1. Start the Emulator

```bash
cd /Users/lorenzomazzocchetti/Desktop/Github/X18_Nintendo_ds/tools/osc-testing
./start-emulator.sh
```

The emulator will start and show:
```
==========================================
X32 Emulator for X18 Mixer OSC Testing
==========================================

Starting X32 emulator...
  IP: 127.0.0.1
  Port: 10024
  Verbose: 1
  ...
```

### 2. Test from macOS (while developing)

From another terminal, use `nc` (netcat) or X32_Command to send OSC messages:

#### Test Basic Connection
```bash
echo -ne "/info\x00" | nc 127.0.0.1 10024
```

#### Set Channel 1 Fader to 50%
```bash
# OSC format: /ch/01/mix/fader ,f 0.5
# Note: Requires binary encoding, easier with X32_Command
```

### 3. Test from 3DS (via Network)

1. Ensure **3DS is on the same WiFi network** as the Mac running the emulator
2. Find the Mac's local IP (not 127.0.0.1):
   ```bash
   ifconfig | grep inet  # Look for 192.168.x.x
   ```
3. In your X18 Mixer 3DS app's OSC code, use that IP address instead of 127.0.0.1

---

## OSC Message Format

### Basic Structure
```
OSC Address | Type Tag | Arguments
/ch/01/mix/fader , f 0.5
```

### Common Messages for X18/X32

**Channel Control:**
```
/ch/01/mix/fader  ,f <value>     # Set fader (0.0 = -∞, 1.0 = +10dB)
/ch/01/mix/on     ,i <0|1>       # Mute (0=on, 1=mute in X32 convention)
/ch/01/config/name ,s <name>     # Set channel name
```

**EQ Control:**
```
/ch/01/eq/1/type  ,i <type>      # EQ band 1 type (0=LCut, 1=LShv, 2=PEQ, etc)
/ch/01/eq/1/f     ,f <freq>      # EQ band 1 frequency (0.0-1.0 normalized)
/ch/01/eq/1/g     ,f <gain>      # EQ band 1 gain (0.0-1.0 normalized)
/ch/01/eq/1/q     ,f <q>         # EQ band 1 Q factor
```

**Query (Request):**
```
/ch/01/mix/fader                 # Query current fader value
/node ,s ch/01/config            # Query channel configuration
```

---

## Emulator Data Persistence

The emulator auto-saves its state when you send `/shutdown`:

```bash
echo -ne "/shutdown\x00" | nc 127.0.0.1 10024
```

This creates an `X32data` file with all current mixer settings. On next startup, it loads automatically.

---

## Testing Checklist

### Phase 1: Connection & Basic Messaging
- [ ] Emulator starts without errors
- [ ] Can query `/info` from emulator
- [ ] Emulator responds with mixer info

### Phase 2: Fader Communication (Send)
- [ ] Send `/ch/01/mix/fader ,f 0.5` from 3DS app
- [ ] Emulator shows command received in verbose log
- [ ] Emulator stores fader value

### Phase 3: Feedback (Receive)
- [ ] Query `/ch/01/mix/fader` from app
- [ ] Emulator returns current value
- [ ] App displays correct dB conversion

### Phase 4: Multi-Channel
- [ ] Send fader updates for channels 2-16
- [ ] All channels respond independently
- [ ] No interference between channels

### Phase 5: EQ Control
- [ ] Send `/ch/01/eq/1/type ,i <value>` 
- [ ] Send `/ch/01/eq/1/f ,f <value>`
- [ ] Send `/ch/01/eq/1/g ,f <value>`
- [ ] Emulator stores EQ settings correctly

---

## Troubleshooting

### Emulator won't compile
```bash
cd /Users/lorenzomazzocchetti/Desktop/Github/X32-Behringer
make clean
make X32
```

### Connection refused on 3DS
- Verify both devices are on the same WiFi network
- Use Mac's local IP (192.168.x.x), not 127.0.0.1
- Check firewall isn't blocking port 10024
- Restart emulator and try again

### OSC messages not showing in emulator log
- Ensure verbose mode is enabled: `/start-emulator.sh` uses `-v 1`
- Check binary encoding of OSC messages (tricky with nc, easier with dedicated tools)

### Emulator crashes
- Check `x32-emulator.log` for error messages
- Try with `-d 1` (debug mode) for more details
- Rebuild if you modified X32 source code

---

## Integration with Development Workflow

### Recommended Setup

1. **Terminal 1**: Run emulator
   ```bash
   cd tools/osc-testing && ./start-emulator.sh
   ```

2. **Terminal 2**: Build 3DS app
   ```bash
   cd /path/to/X18_Nintendo_ds
   make clean && make
   ```

3. **Terminal 3**: (Optional) Tail emulator log
   ```bash
   cd tools/osc-testing
   tail -f x32-emulator.log | grep "ch/01"  # Filter for channel 1
   ```

4. **3DS Emulator/Real Hardware**: Launch your app and test

---

## X32 Emulator Repository

Source: https://github.com/pmaillot/X32-Behringer

The full X32 project includes:
- **X32_Command**: CLI tool for manual OSC testing
- **X32Replay**: Record/playback of OSC sequences
- **X32.c**: Core emulator (what we use)
- Many other utilities for X32 professional use

---

## Next Steps for Full Implementation

1. ✅ Emulator setup complete
2. ⏳ Implement `osc_send()` function in app to send fader/EQ updates
3. ⏳ Implement `osc_receive()` function to handle feedback
4. ⏳ Test bidirectional communication
5. ⏳ Optimize network communication (on-change vs polling)

---

## References

- **OSC Specification**: http://opensoundcontrol.org/
- **X32 OSC Protocol**: https://sites.google.com/site/patrickmaillot/x32
- **X18 Manual**: Check Behringer official documentation
- **3DS libctru SOC Service**: For network I/O on 3DS

---

**Created**: 9 febbraio 2026  
**Last Updated**: 9 febbraio 2026  
**Status**: Testing infrastructure ready for implementation phase
