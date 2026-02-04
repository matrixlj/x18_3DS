#!/bin/bash

# Conversione semplificata ELF -> Binary per 3DS

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}"
cat << "EOF"
╔═══════════════════════════════════════════════════════════╗
║        3DS ELF to Binary Converter                         ║
║        (Supporta Direct Boot su CFW)                       ║
╚═══════════════════════════════════════════════════════════╝
EOF
echo -e "${NC}"

ELF_FILE="build/nds_app.elf"

if [ ! -f "$ELF_FILE" ]; then
    echo -e "${RED}❌ Errore: $ELF_FILE non trovato${NC}"
    exit 1
fi

echo -e "${GREEN}✅ ELF trovato${NC}"

# Converti a binario (formato raw ARM)
echo -e "${YELLOW}📦 Estraendo il codice binario...${NC}"
arm-none-eabi-objcopy -O binary "$ELF_FILE" build/app.bin

# Converti anche il .elf a formato che si può debuggare
arm-none-eabi-objdump -d "$ELF_FILE" > build/app.disasm

if [ -f "build/app.bin" ]; then
    SIZE=$(wc -c < "build/app.bin")
    SIZE_KB=$((SIZE / 1024))
    
    echo -e "${GREEN}✅ Conversione completata!${NC}"
    echo ""
    echo -e "${BLUE}📋 OUTPUT FILES:${NC}"
    echo -e "${YELLOW}  • build/nds_app.elf${NC}     - Executable (per debugging)"
    echo -e "${YELLOW}  • build/app.bin${NC}         - Raw binary ($SIZE_KB KB)"
    echo -e "${YELLOW}  • build/app.disasm${NC}      - Disassembly"
    echo ""
    
    echo -e "${BLUE}📝 OPZIONI PER ESEGUIRE SUL 3DS:${NC}"
    echo ""
    echo -e "${YELLOW}OPZIONE 1: Homebrew Launcher (più facile)${NC}"
    echo -e "  1. Scarica 3dsxtool da: https://github.com/devkitPro/3dsxtool/releases"
    echo -e "  2. Esegui: 3dsxtool build/nds_app.elf build/app.3dsx"
    echo -e "  3. Copia app.3dsx in /3ds/app.3dsx sulla SD card"
    echo -e "  4. Apri Homebrew Launcher sul 3DS"
    echo ""
    
    echo -e "${YELLOW}OPZIONE 2: CIA Install (richiede FBI su 3DS)${NC}"
    echo -e "  1. Scarica bannertool e makerom"
    echo -e "  2. Crea un CIA con: makerom -f cci ..."
    echo -e "  3. Installa con FBI"
    echo ""
    
    echo -e "${YELLOW}OPZIONE 3: Direct Boot (se supportato dal CFW)${NC}"
    echo -e "  1. Copia build/app.bin in: /boot.bin sulla SD root"
    echo -e "  2. Riavvia il 3DS"
    echo ""
    
    echo -e "${BLUE}🔍 INFO DEL BINARIO:${NC}"
    echo -e "  Size: $SIZE bytes ($SIZE_KB KB)"
    echo -e "  Entrypoint: 0x08000000"
    echo -e "  Arch: ARM32 (ARMv5TE)"
    echo ""
    
    ls -lh build/app.bin build/nds_app.elf
else
    echo -e "${RED}❌ Errore nella conversione a binario${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Completato!${NC}"
