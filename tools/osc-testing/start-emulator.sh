#!/bin/bash

# X32 Emulator Launcher for X18 Mixer 3DS OSC Testing
# This script starts the X32 emulator which serves as a testing tool
# for OSC communication (X32 and X18 use compatible OSC protocols)

# Configuration
EMULATOR_PATH="/Users/lorenzomazzocchetti/Desktop/Github/X32-Behringer/build/X32"
LOG_FILE="./x32-emulator.log"
IP_ADDRESS="auto"        # Auto-detect from system network
PORT=10023               # X32 standard OSC port (X18 also uses 10024, but emulator defaults to 10023)
VERBOSE=1                # 1 = verbose mode (see all OSC commands)
DEBUG=0                  # 1 = debug mode

echo "=========================================="
echo "X32 Emulator for X18 Mixer OSC Testing"
echo "=========================================="
echo ""
echo "Starting X32 emulator..."
echo "  IP: $IP_ADDRESS"
echo "  Port: $PORT"
echo "  Verbose: $VERBOSE"
echo "  Log file: $LOG_FILE"
echo ""
echo "Commands:"
echo "  - Query: /info"
echo "  - Set fader: /ch/01/mix/fader ,f 0.5"
echo "  - Query fader: /ch/01/mix/fader"
echo "  - Shutdown: /shutdown"
echo ""
echo "=========================================="
echo ""

# Check if emulator exists
if [ ! -f "$EMULATOR_PATH" ]; then
    echo "ERROR: X32 emulator not found at $EMULATOR_PATH"
    echo "Please build it with: cd /Users/lorenzomazzocchetti/Desktop/Github/X32-Behringer && make X32"
    exit 1
fi

# Start emulator with specified options
# Note: -i option will be overridden by the system's default IP
$EMULATOR_PATH -d $DEBUG -v $VERBOSE 2>&1 | tee "$LOG_FILE"
