#!/bin/bash

# Setup environment per sviluppo 3DS

export DEVKITPRO=/Users/lorenzomazzocchetti/Desktop/Github/X18_Nintendo_ds/libctru
export DEVKITARM=/usr/local
export PATH=$DEVKITARM/bin:$PATH

echo "✅ Environment configurato:"
echo "DEVKITPRO=$DEVKITPRO"
echo "DEVKITARM=$DEVKITARM"
echo ""
echo "Include path: $DEVKITPRO/libctru/include"
echo "Compilatore: $(which arm-none-eabi-gcc)"
