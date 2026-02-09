# OSC Implementation Strategy for X18 Mixer 3DS

**Status**: Planning phase  
**Date**: 9 febbraio 2026  
**Testing Tool**: X32 Emulator (192.168.x.x:10024)  

---

## Architecture Overview

```
┌─────────────────────────────────────────────────┐
│         X18 Mixer 3DS Application               │
├─────────────────────────────────────────────────┤
│                                                 │
│  ┌────────────────────────────────────────┐    │
│  │     UI Layer (Touch + Rendering)       │    │
│  │  • render_bot_screen() - mixer display │    │
│  │  • update_mixer_touch() - touch input   │    │
│  └────────────────────────────────────────┘    │
│                    ↓                            │
│  ┌────────────────────────────────────────┐    │
│  │    OSC Communication Layer (NEW)       │    │
│  │  • osc_init() - UDP socket setup       │    │
│  │  • osc_send_fader() - send fader value │    │
│  │  • osc_send_mute() - send mute state   │    │
│  │  • osc_send_eq() - send EQ parameters  │    │
│  │  • osc_receive() - listen for feedback │    │
│  │  • osc_shutdown() - cleanup            │    │
│  └────────────────────────────────────────┘    │
│                    ↓ UDP socket (10024)         │
└─────────────────────────────────────────────────┘
              Network / Emulator
                     ↓
┌─────────────────────────────────────────────────┐
│     X32 Emulator / Real X18 Hardware            │
│                                                 │
│  • Listens on port 10024 (UDP)                  │
│  • Parses OSC messages                          │
│  • Updates mixer state                          │
│  • Sends feedback (optional)                    │
└─────────────────────────────────────────────────┘
```

---

## Phase 1: Core OSC Send (v0.1.3)

### Goal
Enable the app to send fader, mute, and EQ updates to X18/X32 mixer via OSC.

### Implementation Tasks

#### Task 1.1: UDP Socket Initialization
**File**: `src/main.c`  
**Function**: `osc_init(const char* host, int port)`

```c
// Open UDP socket
// Resolve hostname to IP (127.0.0.1 for testing, mixer IP for real use)
// Store socket descriptor in global state

int osc_socket = -1;  // Global for OSC socket
struct sockaddr_in mixer_addr;  // Target mixer address
```

**Dependencies**:
- libctru SOC service (already available in libctru)
- `arpa/inet.h`, `sys/socket.h`

**Testing**:
- Start emulator
- Call `osc_init("127.0.0.1", 10024)` from debug menu
- Verify connection (emulator logs should show nothing yet, but connection accepted)

---

#### Task 1.2: Send Fader OSC Messages
**File**: `src/main.c`  
**Function**: `osc_send_fader(int channel, float value)`

**OSC Message Format**:
```
/ch/XX/mix/fader ,f <value>
```

**Implementation**:
```c
void osc_send_fader(int channel, float value) {
    // Build OSC message:
    // 1. Address: "/ch/XX/mix/fader"
    // 2. Type tag: ",f"
    // 3. Float data: value (0.0-1.0)
    
    // Send via UDP socket
    // Log: "[OSC] CH%02d fader: %.2f", channel+1, value
}
```

**Call Location**:
- In `update_mixer_touch()` after fader value is updated
- Or: in `render_bot_screen()` if implementing continuous polling

**Testing**:
```bash
# Start emulator
cd tools/osc-testing && ./start-emulator.sh

# Move faders in app
# Check log for: ->X,   XX B: /ch/01/mix/fader~~~~,f~~[value]
```

---

#### Task 1.3: Send Mute OSC Messages
**File**: `src/main.c`  
**Function**: `osc_send_mute(int channel, int on)`

**OSC Message Format**:
```
/ch/XX/mix/on ,i <on>
```

**Implementation**:
```c
void osc_send_mute(int channel, int on) {
    // Build OSC message:
    // 1. Address: "/ch/XX/mix/on"
    // 2. Type tag: ",i"
    // 3. Int data: on (0=muted, 1=unmuted)
    
    // Send via UDP socket
    // Log: "[OSC] CH%02d mute: %s", channel+1, (on ? "off" : "on")
}
```

**Call Location**:
- In `update_mixer_touch()` when mute button is pressed

**Testing**:
```bash
# Tap mute button on each channel
# Check log for: ->X,   XX B: /ch/XX/mix/on~~~~,i~~[0|1]
```

---

#### Task 1.4: Send EQ OSC Messages
**File**: `src/main.c`  
**Function**: `osc_send_eq(int channel, int band, const eq_band_t* band_data)`

**OSC Message Format** (multiple messages per band):
```
/ch/XX/eq/B/type ,i <type>
/ch/XX/eq/B/f    ,f <frequency>
/ch/XX/eq/B/g    ,f <gain>
/ch/XX/eq/B/q    ,f <q>
```

**Implementation**:
```c
void osc_send_eq(int channel, int band, const eq_band_t* band) {
    // Send 4 messages for each EQ band:
    // 1. Type: /ch/XX/eq/B/type ,i band->type
    // 2. Frequency: /ch/XX/eq/B/f ,f band->freq
    // 3. Gain: /ch/XX/eq/B/g ,f band->gain
    // 4. Q: /ch/XX/eq/B/q ,f band->q
    
    // Send via UDP socket
    // Log: "[OSC] CH%02d EQ%d type=%d freq=%.2f gain=%.2f q=%.2f"
}
```

**Call Location**:
- In `update_eq_touch()` after any EQ parameter change

**Testing**:
```bash
# Adjust each EQ parameter
# Check log for all 4 message types sent per band
```

---

### Integration with Existing Code

**Modify `update_mixer_touch()`**:
```c
// After fader value is updated:
if (fader value changed) {
    osc_send_fader(channel, new_value);  // NEW
}

// After mute button is pressed:
if (mute status changed) {
    osc_send_mute(channel, new_status);  // NEW
}
```

**Modify `update_eq_touch()`** (when created):
```c
// After EQ parameter is changed:
if (eq parameter changed) {
    osc_send_eq(channel, band_index, &band_data);  // NEW
}
```

---

## Phase 2: OSC Feedback Reception (v0.1.4)

### Goal
Listen for OSC feedback from the mixer and update app state accordingly.

### Implementation

**Function**: `osc_receive()` (non-blocking)

```c
void osc_receive(void) {
    // Check for incoming messages (non-blocking)
    // Parse OSC message
    // Update app state based on message
    //   e.g., /ch/01/mix/fader -> update g_faders[0].value
    // No UI updates here (let render loop handle that)
}
```

**Call Location**: In main game loop, like `update_mixer_touch()` is called

**Feedback Strategy Decision**:
- **Option A** (Polling): App queries mixer state every frame (wasteful)
- **Option B** (Subscriptions): Mixer sends updates on state changes (better)
- **Option C** (Hybrid): Listen for feedback, but don't depend on it (safer)

**Recommendation**: Option C for initial implementation
- Send commands from app
- Listen for feedback in background
- Display local state (match feedback when it arrives)
- Handle latency gracefully

---

## Phase 3: Optimization (v0.1.5)

### Considerations

1. **Bandwidth**: Send only on-change, not every frame
   - Current: 16 faders × 60 FPS = 960 messages/sec = TOO MUCH
   - Optimized: 16 faders × 1 message per change = ~16 messages/sec
   
2. **Latency**: UDP might drop packets
   - Solution: Resend critical changes (mute, fader > 3dB change)
   - Keep local state as source of truth initially
   
3. **Buffering**: Bundle multiple OSC messages
   - Emulator can handle, but not essential for MVP

4. **Error Recovery**: What if mixer becomes unavailable?
   - Graceful degradation: Continue working locally
   - Log errors but don't crash

---

## Development Checklist

### Phase 1 Tasks (MVP)
- [ ] Task 1.1: UDP socket initialization with osc_init()
- [ ] Task 1.2: Fader OSC send with osc_send_fader()
- [ ] Task 1.3: Mute OSC send with osc_send_mute()
- [ ] Task 1.4: EQ OSC send with osc_send_eq()
- [ ] Integration: Hook up to existing touch/input code
- [ ] Testing: Connect to emulator and verify all messages

### Phase 2 Tasks (Feedback)
- [ ] Implement osc_receive() for packet listening
- [ ] Parse incoming OSC messages
- [ ] Update app state from feedback
- [ ] Test bidirectional communication

### Phase 3 Tasks (Optimization)
- [ ] Implement on-change sending (not polling)
- [ ] Add message buffering/batching
- [ ] Handle packet loss and retries
- [ ] Test with real X18 hardware

---

## Testing Strategy

### Test 1: Connection Established
```
✓ Start emulator
✓ Run osc_init("127.0.0.1", 10024)
✓ Log should show connection accepted
```

### Test 2: Send Fader (Single Channel)
```
✓ Move fader for channel 1
✓ Emulator log shows: /ch/01/mix/fader ,f X.XXXX
✓ Value matches app fader position (0.0-1.0)
```

### Test 3: Send Fader (All Channels)
```
✓ Move faders for all 16 channels
✓ All channels respond independently
✓ No cross-talk or interference
```

### Test 4: Send Mute
```
✓ Tap mute button on channel 1
✓ Emulator log shows: /ch/01/mix/on ,i [0|1]
✓ Multiple mutes work correctly
```

### Test 5: Send EQ
```
✓ Adjust EQ parameters for channel 1, band 1
✓ Emulator log shows 4 messages: /ch/01/eq/1/type, f, g, q
✓ Values are normalized (0.0-1.0)
```

### Test 6: Real Hardware (Final)
```
✓ Connect 3DS to X18 via same WiFi
✓ Use X18 IP instead of 127.0.0.1
✓ Send fader updates
✓ Verify X18 display updates
✓ Move X18 faders and see app feedback
```

---

## Code Architecture Decisions

### Where to put OSC functions?

**Option A**: Separate file `src/osc.c` + `src/osc.h`
- Pros: Modular, easy to maintain, can test independently
- Cons: New file management, potential circular dependencies

**Option B**: Add to existing `src/main.c`
- Pros: Simple, all game logic in one place
- Cons: File gets large, harder to test separately

**Recommendation**: **Option B for MVP** (simplicity)
- Start in main.c
- Extract to separate file if it grows > 500 lines
- Current main.c is 2274 lines anyway

### Global State for OSC

```c
// In main.c, near top with other globals
int g_osc_socket = -1;
struct sockaddr_in g_mixer_addr = {0};
char g_mixer_host[16] = "127.0.0.1";      // Default: localhost
int g_mixer_port = 10024;                 // X18/X32 standard port
int g_osc_connected = 0;
```

### Error Handling

```c
// Graceful degradation model
if (osc_send_fader(ch, val) < 0) {
    // Log error, but don't crash
    // App still works with local state
    // Retry on next change
}
```

---

## Resources & References

### Documentation
- [TEST_SAMPLES.md](TEST_SAMPLES.md) - Example OSC messages to send
- [TESTING_GUIDE.md](TESTING_GUIDE.md) - How to run emulator
- X32 OSC Spec: https://sites.google.com/site/patrickmaillot/x32

### Code Examples
- X32-Behringer X32_Command.c - How to send/receive OSC
- libctru examples for networking

### Tools
- X32 Emulator: `/Users/lorenzomazzocchetti/Desktop/Github/X32-Behringer/build/X32`
- X32_Command: For manual debugging

---

## Milestones

| Milestone | Target | Status |
|-----------|--------|--------|
| v0.1.3 - OSC Send MVP | 1 week | 📅 Planned |
| v0.1.4 - OSC Feedback | 2 weeks | 📅 Planned |
| v0.1.5 - Optimization | 3 weeks | 📅 Planned |
| v0.2.0 - Real Hardware | 4 weeks | 📅 Planned |

---

**Author**: AI Assistant  
**Date Created**: 9 febbraio 2026  
**Last Modified**: 9 febbraio 2026
