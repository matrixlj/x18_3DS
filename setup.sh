#!/bin/bash

# Setup environment per sviluppo 3DS

export DEVKITPRO=/opt/devkitpro
export DEVKITARM=$DEVKITPRO/devkitARM
export PATH=$DEVKITARM/bin:$PATH

echo "✅ Environment configurato:"
echo "DEVKITPRO=$DEVKITPRO"
echo "DEVKITARM=$DEVKITARM"
echo ""
echo "3ds_rules path: $DEVKITARM/3ds_rules"
echo "Compilatore: $(which arm-none-eabi-gcc)"
