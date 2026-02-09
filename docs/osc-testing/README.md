# OSC Testing Infrastructure

Complete setup for testing OSC (Open Sound Control) communication with X32/X18 mixers.

## Quick Links

- **🚀 [Getting Started](TESTING_GUIDE.md)** - Start the emulator and run basic tests
- **📝 [Test Samples](TEST_SAMPLES.md)** - Example OSC commands and message formats
- **🏗️ [Implementation Strategy](IMPLEMENTATION_STRATEGY.md)** - Detailed development plan for OSC integration

## What's Included

### `/tools/osc-testing/`

**Start Emulator Script**
```bash
./start-emulator.sh
```
Launches the X32 emulator and logs all OSC traffic.

### `/docs/osc-testing/`

| Document | Purpose |
|----------|---------|
| **TESTING_GUIDE.md** | How to set up and use the emulator for testing |
| **TEST_SAMPLES.md** | Concrete examples of OSC messages you can send |
| **IMPLEMENTATION_STRATEGY.md** | Detailed roadmap for adding OSC to the app |

## Fast Start

### 1️⃣ Compile the Emulator (one-time setup)

```bash
cd /Users/lorenzomazzocchetti/Desktop/Github/X32-Behringer
make clean && make X32
# Output: build/X32 (1.5MB executable)
```

### 2️⃣ Start the Emulator

```bash
cd /Users/lorenzomazzocchetti/Desktop/Github/X18_Nintendo_ds/tools/osc-testing
./start-emulator.sh

# Output:
# ==========================================
# X32 Emulator for X18 Mixer OSC Testing
# ==========================================
# Starting X32 emulator...
#   IP: 127.0.0.1
#   Port: 10024
#   ...
```

### 3️⃣ Send Test Commands

From another terminal, send OSC commands:

```bash
# Method 1: Using nc (netcat) - simple but requires binary encoding
echo -ne "/info\x00\x00\x00" | nc 127.0.0.1 10024

# Method 2: Using X32_Command - easier, more visual
cd /Users/lorenzomazzocchetti/Desktop/Github/X32-Behringer/build
./X32_Command -i 127.0.0.1
# Then type: /info
```

### 4️⃣ Check the Logs

The emulator logs all OSC traffic in `x32-emulator.log`:

```bash
tail -f ./x32-emulator.log
```

## Why This Works for X18

**The X32 and X18 share the same OSC protocol** for the most important features:

| Feature | X32 | X18 | Status |
|---------|-----|-----|--------|
| Channel Faders | ✅ | ✅ | Same command format |
| Mute/Unmute | ✅ | ✅ | Same command format |
| EQ Control | ✅ | ✅ | Same command format |
| Advanced FX | 🟡 | ❌ | Not needed for v1 |

**Port Number**: 
- X32 uses port **10023**
- X18 uses port **10024** (what we test with)
- Emulator defaults to **10024**

## Typical Workflow

```
Terminal 1: Emulator           Terminal 2: 3DS App Dev       Terminal 3: Log Monitoring
┌──────────────────┐          ┌──────────────────┐         ┌──────────────────────┐
│ ./start-emulator │          │ make clean       │         │ tail -f *.log        │
│                  │          │ make             │         │                      │
│ [Listening...]   │◄─────────│ Launch in 3DS    │─────────►│ /ch/01/mix/fader ?f  │
│                  │          │ emulator         │         │ /ch/01/mix/fader ,f  │
│                  │          │ Move faders      │         │ 0.5000               │
└──────────────────┘          └──────────────────┘         └──────────────────────┘
```

## Development Phases

### Phase 1: OSC Send (v0.1.3)
- Implement `osc_send_fader()` - send fader updates
- Implement `osc_send_mute()` - send mute state
- Implement `osc_send_eq()` - send EQ parameters
- **Test**: Emulator receives all messages correctly

### Phase 2: OSC Feedback (v0.1.4)
- Implement `osc_receive()` - listen for emulator responses
- Handle bidirectional communication
- **Test**: App updates when mixer state changes

### Phase 3: Real Hardware (v0.2.0)
- Replace 127.0.0.1 with real X18 IP
- Test with physical X18 mixer
- Optimize network communication

## Common Issues & Solutions

| Issue | Solution |
|-------|----------|
| Emulator won't compile | `cd X32-Behringer && make clean && make X32` |
| Connection refused on 3DS | Use Mac's local IP (192.168.x.x), not 127.0.0.1 |
| No OSC messages in log | Check verbose mode is on in start-emulator.sh |
| Slow emulator startup | Normal - can take 5-10 seconds |
| Port 10024 already in use | Check if another instance is running |

## File Structure

```
X18_Nintendo_ds/
├── tools/osc-testing/
│   └── start-emulator.sh          ← Start the emulator here
│
└── docs/osc-testing/
    ├── README.md                  ← You are here
    ├── TESTING_GUIDE.md           ← How to test
    ├── TEST_SAMPLES.md            ← Example messages
    └── IMPLEMENTATION_STRATEGY.md ← Development roadmap
```

## Key Concepts

### OSC Message Format

```
/ch/01/mix/fader ,f 0.5
 └─ Address      └─ Type (f=float, i=int, s=string)
                 └─ Value
```

### Value Ranges

- **Fader**: 0.0 = -∞dB, 0.76 ≈ 0dB, 1.0 = +10dB
- **Mute**: 0 = muted, 1 = unmuted
- **Frequency**: 0.0-1.0 (normalized), maps to Hz internally
- **Gain**: -1.0 to +1.0 (negative = cutting, positive = boosting)

### Channel Numbering

**Important**: Use zero-padded format
- ✅ Correct: `/ch/01`, `/ch/02`, ..., `/ch/16`
- ❌ Wrong: `/ch/1`, `/ch/2`, ..., `/ch/16`

## Next Steps

1. **Read** [TESTING_GUIDE.md](TESTING_GUIDE.md) for detailed instructions
2. **Start** the emulator: `./start-emulator.sh`
3. **Send** test commands: See [TEST_SAMPLES.md](TEST_SAMPLES.md)
4. **Plan** implementation: Check [IMPLEMENTATION_STRATEGY.md](IMPLEMENTATION_STRATEGY.md)
5. **Code** OSC functions (when ready): Follow the implementation plan

## Resources

- **X32 Emulator**: https://github.com/pmaillot/X32-Behringer
- **OSC Specification**: http://opensoundcontrol.org/
- **X32 OSC Protocol**: https://sites.google.com/site/patrickmaillot/x32
- **3DS libctru Networking**: Available in libctru documentation

---

**Status**: ✅ Testing infrastructure complete and ready  
**Created**: 9 febbraio 2026  
**Last Updated**: 9 febbraio 2026
