# OSC Configuration for X18 Mixer Testing and Deployment

This file contains the main configuration settings for OSC communication.
Modify these values for different testing scenarios.

## Network Configuration

### Development Testing (Emulator)
```
MIXER_HOST=127.0.0.1
MIXER_PORT=10024
MIXER_TYPE=emulator     # "emulator" or "real_hardware"
```

### Real Hardware Testing
```
MIXER_HOST=192.168.1.100    # Replace with your X18's IP
MIXER_PORT=10024
MIXER_TYPE=real_hardware
```

### Settings Explanation

**MIXER_HOST**: 
- `127.0.0.1` = localhost (testing on same machine)
- `192.168.x.x` = your X18/X32 IP on network

**MIXER_PORT**:
- `10024` = X18 standard port
- `10023` = X32 standard port (for compatibility testing)

**MIXER_TYPE**:
- `emulator` = Safe testing, all responses guaranteed
- `real_hardware` = Real X18, needs network connection

## OSC Message Configuration

### Fader Update Strategy

```
SEND_ON_CHANGE=1        # 1 = send only when value changes
                        # 0 = send every frame (wasteful)

FADER_CHANGE_THRESHOLD=0.01  # Minimum change to trigger send
                             # Prevents tiny movements from spamming

POLLING_INTERVAL_MS=50  # If SEND_ON_CHANGE=0, send this often (ms)
```

### EQ Update Strategy

```
SEND_EQ_ON_CHANGE=1     # Send EQ only when parameters change
EQ_CHANGE_THRESHOLD=0.02    # Minimum change for EQ parameters
```

### Mute Updates

```
SEND_MUTE_IMMEDIATELY=1 # Always send mute changes immediately
                        # (mute is binary, no threshold needed)
```

## Connection & Timeout Settings

```
CONNECT_TIMEOUT_MS=5000     # How long to wait for connection
SOCKET_TIMEOUT_MS=100       # How long to wait for response
SEND_RETRY_COUNT=3          # Retry failed sends N times
SEND_RETRY_DELAY_MS=50      # Wait this long between retries
```

## Logging & Debugging

```
VERBOSE_LOGGING=1           # 1 = log all OSC traffic to console
                            # 0 = only log errors

LOG_RX_MESSAGES=1           # Log received (feedback) messages
LOG_TX_MESSAGES=1           # Log transmitted messages

DEBUG_MODE=0                # 1 = print detailed debug info
                            # 0 = normal operation
```

## Performance Tuning

```
UDP_BUFFER_SIZE=1024        # Size of UDP receive buffer (bytes)
MESSAGE_BATCH_SIZE=10       # Bundle up to N messages before sending
BATCH_TIMEOUT_MS=10         # Or send batch after this many ms
```

## Emulator Special Settings

These only apply when MIXER_TYPE=emulator:

```
EMULATOR_SAVE_ON_SHUTDOWN=1    # Persist emulator state to X32data file
EMULATOR_VERBOSE_LOGGING=1     # Show ALL OSC traffic in emulator
EMULATOR_ECHO_XREMOTE=0        # Echo xremote subscriptions
```

## Channel Configuration

```
NUM_CHANNELS=16             # How many channels to control
FIRST_CHANNEL=1             # First channel number (usually 1)

# Channel names (optional, for logging)
CHANNEL_NAMES="Ch1,Ch2,Ch3,Ch4,Ch5,Ch6,Ch7,Ch8,Ch8,Ch10,Ch11,Ch12,Ch13,Ch14,Ch15,Ch16"
```

## Feature Flags

```
ENABLE_FADER_CONTROL=1      # Allow sending fader updates
ENABLE_MUTE_CONTROL=1       # Allow sending mute/unmute
ENABLE_EQ_CONTROL=1         # Allow sending EQ parameters
ENABLE_FEEDBACK=1           # Listen for mixer feedback
ENABLE_NETWORK=1            # Enable all network communication
```

## Version & Compatibility

```
OSC_PROTOCOL_VERSION=1.0    # Implemented protocol version
X18_COMPATIBLE=1            # Tested with X18
X32_COMPATIBLE=1            # Tested with X32
MINIMUM_FW_VERSION=2.0      # Minimum mixer firmware version
```

## Default Values (Fallback)

If this file is not found, app uses these defaults:

```c
#define DEFAULT_MIXER_HOST      "127.0.0.1"
#define DEFAULT_MIXER_PORT      10024
#define DEFAULT_CONNECT_TIMEOUT 5000  // milliseconds
#define DEFAULT_NUM_CHANNELS    16
#define DEFAULT_VERBOSE_LOGGING 1
```

---

## How to Use This File

### In Development

1. Keep defaults for emulator testing
2. Modify only MIXER_HOST and MIXER_PORT
3. Set VERBOSE_LOGGING=1 to debug

### Before Real Hardware Testing

1. Change MIXER_HOST to X18 IP address
2. Change MIXER_TYPE to "real_hardware"
3. Test with small changes first (single fader)
4. Monitor for any errors or timeouts

### Performance Optimization

1. Set SEND_ON_CHANGE=1 and adjust FADER_CHANGE_THRESHOLD
2. Monitor CPU/bandwidth usage
3. Increase thresholds if updates are too frequent
4. Decrease if responsiveness suffers

---

## File Format Notes

- Lines starting with `#` are comments
- Format: `KEY=VALUE`
- No spaces around `=`
- String values don't need quotes
- Empty lines are ignored

## Example Scenarios

### Scenario 1: Safe Emulator Testing
```
MIXER_HOST=127.0.0.1
MIXER_PORT=10024
MIXER_TYPE=emulator
VERBOSE_LOGGING=1
DEBUG_MODE=1
SEND_ON_CHANGE=0  # Send every frame for testing
```

### Scenario 2: Real Hardware (Optimized)
```
MIXER_HOST=192.168.1.100
MIXER_PORT=10024
MIXER_TYPE=real_hardware
VERBOSE_LOGGING=0
SEND_ON_CHANGE=1
FADER_CHANGE_THRESHOLD=0.01
```

### Scenario 3: Performance Testing
```
MIXER_HOST=127.0.0.1
MESSAGE_BATCH_SIZE=20
BATCH_TIMEOUT_MS=20
VERBOSE_LOGGING=0
```

---

**Created**: 9 febbraio 2026  
**Version**: 1.0  
**Status**: Ready for implementation phase
