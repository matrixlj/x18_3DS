#!/bin/bash

# Script per caricare l'app 3DSX sul 3DS via SD card

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}"
cat << "EOF"
╔═══════════════════════════════════════════════════════════╗
║          3DSX Deployment Tool                              ║
║          (Deploy to 3DS via SD Card)                       ║
╚═══════════════════════════════════════════════════════════╝
EOF
echo -e "${NC}"

# Variabili
THREEDSX_FILE="build/app.3dsx"
SD_MOUNT_POINT="/Volumes/3DS"
SD_APP_DIR="$SD_MOUNT_POINT/3ds"

# Verifica che il file esista
if [ ! -f "$THREEDSX_FILE" ]; then
    echo -e "${RED}❌ File 3DSX non trovato: $THREEDSX_FILE${NC}"
    echo -e "${YELLOW}Esegui: python3 create_3dsx.py build/nds_app.elf build/app.3dsx${NC}"
    exit 1
fi

FILE_SIZE=$(ls -lh "$THREEDSX_FILE" | awk '{print $5}')
echo -e "${GREEN}✅ 3DSX trovato: $THREEDSX_FILE ($FILE_SIZE)${NC}"
echo ""

# Verifica che la SD card sia montata
if [ ! -d "$SD_MOUNT_POINT" ]; then
    echo -e "${RED}❌ SD card non montata su: $SD_MOUNT_POINT${NC}"
    echo ""
    echo -e "${YELLOW}📋 Passaggi:${NC}"
    echo "  1. Connetti la SD card al computer"
    echo "  2. Attendi il mount automatico"
    echo "  3. Riprova lo script"
    echo ""
    echo -e "${YELLOW}Or, monta manualmente:${NC}"
    echo "  diskutil list  # Identifica il disco"
    echo "  diskutil mount diskXsY  # Monta la partizione"
    exit 1
fi

echo -e "${GREEN}✅ SD card montata: $SD_MOUNT_POINT${NC}"

# Crea la cartella /3ds se non esiste
if [ ! -d "$SD_APP_DIR" ]; then
    echo -e "${YELLOW}📁 Creando cartella /3ds...${NC}"
    mkdir -p "$SD_APP_DIR"
fi

# Copia il file
DEST_FILE="$SD_APP_DIR/app.3dsx"
echo -e "${YELLOW}📦 Copiando file...${NC}"

cp "$THREEDSX_FILE" "$DEST_FILE"

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✅ File copiato con successo!${NC}"
    echo ""
    echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║  PROSSIMI STEP SUL 3DS                 ║${NC}"
    echo -e "${BLUE}╠════════════════════════════════════════╣${NC}"
    echo -e "${YELLOW}1. Espelli la SD card dal computer${NC}"
    echo -e "${YELLOW}2. Inserisci la SD card nel 3DS${NC}"
    echo -e "${YELLOW}3. Dalla Home, apri l'Homebrew Launcher${NC}"
    echo -e "${YELLOW}   (o accedi dal menu browser)${NC}"
    echo -e "${YELLOW}4. Cerca e premi A su: 'app'${NC}"
    echo -e "${YELLOW}5. Il programma dovrebbe avviarsi!${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
    echo ""
    
    echo -e "${GREEN}📋 Info file:${NC}"
    ls -lh "$DEST_FILE"
    echo ""
    
    echo -e "${YELLOW}💡 Troubleshooting:${NC}"
    echo "  • Se non compare in Homebrew Launcher: verifica /3ds/app.3dsx"
    echo "  • Se crash al avvio: controlla il codice in src/main.c"
    echo "  • Per debug: visualizza build/app.disasm"
else
    echo -e "${RED}❌ Errore nella copia del file${NC}"
    exit 1
fi

# Suggerimento per smontare
echo ""
echo -e "${YELLOW}🔧 Per smontare la SD card:${NC}"
echo "  diskutil unmount $SD_MOUNT_POINT"
echo ""
