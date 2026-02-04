#!/bin/bash

# Script per testare l'app 3DSX su Citra Emulator

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}"
cat << "EOF"
╔═══════════════════════════════════════════════════════════╗
║          3DSX Emulation Tester (Citra)                     ║
╚═══════════════════════════════════════════════════════════╝
EOF
echo -e "${NC}"

# Variabili
THREEDSX_FILE="build/app.3dsx"
CITRA_APP="/Applications/Citra.app"
CITRA_BIN="$CITRA_APP/Contents/MacOS/citra"

# Verifica 3DSX
if [ ! -f "$THREEDSX_FILE" ]; then
    echo -e "${RED}❌ File 3DSX non trovato: $THREEDSX_FILE${NC}"
    echo -e "${YELLOW}Esegui: make 3dsx${NC}"
    exit 1
fi

echo -e "${GREEN}✅ 3DSX trovato: $THREEDSX_FILE${NC}"
echo ""

# Verifica Citra
if [ ! -f "$CITRA_BIN" ]; then
    echo -e "${RED}❌ Citra non installato su: $CITRA_APP${NC}"
    echo ""
    echo -e "${YELLOW}Opzioni di installazione:${NC}"
    echo ""
    echo -e "${YELLOW}1. Via Homebrew:${NC}"
    echo "   brew install citra"
    echo ""
    echo -e "${YELLOW}2. Manualmente:${NC}"
    echo "   • Visita: https://github.com/citra-emu/citra/releases"
    echo "   • Scarica: citra-osx-universal.dmg"
    echo "   • Installa in /Applications/Citra.app"
    echo ""
    echo -e "${YELLOW}3. Citra Nightly (versione in sviluppo):${NC}"
    echo "   brew install citra-nightly"
    echo ""
    exit 1
fi

echo -e "${GREEN}✅ Citra trovato: $CITRA_APP${NC}"
echo ""

# Avvia Citra con il 3DSX
echo -e "${YELLOW}🎮 Avviando Citra con l'app...${NC}"
echo "   File: $THREEDSX_FILE"
echo ""

if [ -x "$CITRA_BIN" ]; then
    # Avvia in background
    open -a Citra "$THREEDSX_FILE" &
    sleep 2
    
    echo -e "${GREEN}✅ Citra avviato!${NC}"
    echo ""
    echo -e "${BLUE}💡 CONTROLLI EMULATOR:${NC}"
    echo "  D-Pad        → Arrow keys"
    echo "  A/B/X/Y      → Z, X, Q, W"
    echo "  L/R          → A, S"
    echo "  Start/Select → Return, Backspace"
    echo "  Home         → Esc"
    echo ""
    echo -e "${BLUE}🎯 OPZIONI:${NC}"
    echo "  • Puoi regolare velocità in Settings"
    echo "  • Usa Save States per debug veloce"
    echo "  • GDB debugger disponibile (Settings > Debug)"
    echo ""
else
    echo -e "${RED}❌ Citra non è eseguibile${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Testing avviato!${NC}"
