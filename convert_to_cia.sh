#!/bin/bash

# Script per convertire ELF a 3DSX per 3DS

# Colori
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}=== 3DS ELF to 3DSX Converter ===${NC}"

# Variabili
ELF_FILE="build/nds_app.elf"
ICON_FILE="build/icon.bin"
BANNER_FILE="build/banner.bin"
OUTPUT_3DSX="build/app.3dsx"

# Verifica che l'ELF esista
if [ ! -f "$ELF_FILE" ]; then
    echo -e "${RED}❌ File ELF non trovato: $ELF_FILE${NC}"
    exit 1
fi

echo -e "${GREEN}✅ ELF trovato: $ELF_FILE${NC}"

# Crea file dummy per icon e banner se non esistono
if [ ! -f "$ICON_FILE" ]; then
    echo -e "${YELLOW}📝 Creando file icon dummy...${NC}"
    dd if=/dev/zero of="$ICON_FILE" bs=1 count=512 2>/dev/null
fi

if [ ! -f "$BANNER_FILE" ]; then
    echo -e "${YELLOW}📝 Creando file banner dummy...${NC}"
    dd if=/dev/zero of="$BANNER_FILE" bs=1 count=2048 2>/dev/null
fi

echo -e "${YELLOW}📦 Convertendo ELF a binario...${NC}"

# Converti a binario
arm-none-eabi-objcopy -O binary "$ELF_FILE" build/app.bin

if [ ! -f "build/app.bin" ]; then
    echo -e "${RED}❌ Errore nella conversione a binario${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Binary creato: build/app.bin${NC}"

# Scarica 3dsxtool
echo -e "${YELLOW}📥 Cercando 3dsxtool...${NC}"

if ! command -v 3dsxtool &> /dev/null; then
    echo -e "${YELLOW}Installando 3dsxtool...${NC}"
    
    # Scarica versione x86_64
    DOWNLOAD_URL="https://github.com/devkitPro/3dsxtool/releases/download/v1.2/3dsxtool-1.2-macos.zip"
    
    curl -L -o /tmp/3dsxtool.zip "$DOWNLOAD_URL" 2>/dev/null
    
    if [ -f /tmp/3dsxtool.zip ]; then
        unzip -q /tmp/3dsxtool.zip -d /tmp/
        if [ -f /tmp/3dsxtool ]; then
            sudo mv /tmp/3dsxtool /usr/local/bin/
            sudo chmod +x /usr/local/bin/3dsxtool
            rm /tmp/3dsxtool.zip
        fi
    fi
fi

# Crea 3DSX
if command -v 3dsxtool &> /dev/null; then
    echo -e "${YELLOW}Creando 3DSX...${NC}"
    3dsxtool "$ELF_FILE" "$OUTPUT_3DSX" -i "$ICON_FILE" 2>&1
    
    if [ -f "$OUTPUT_3DSX" ]; then
        echo -e "${GREEN}✅ File 3DSX creato con successo!${NC}"
        echo -e "${GREEN}📁 Posizione: $OUTPUT_3DSX${NC}"
        ls -lh "$OUTPUT_3DSX"
        echo ""
        echo -e "${GREEN}╔════════════════════════════════════════╗${NC}"
        echo -e "${GREEN}║  ISTRUZIONI PER 3DS SBLOCCATO (CFW)   ║${NC}"
        echo -e "${GREEN}╠════════════════════════════════════════╣${NC}"
        echo -e "${YELLOW}1. Connetti la SD card al computer${NC}"
        echo -e "${YELLOW}2. Copia: $OUTPUT_3DSX${NC}"
        echo -e "${YELLOW}   In: /3ds/app.3dsx${NC}"
        echo -e "${YELLOW}3. Inserisci la SD card nel 3DS${NC}"
        echo -e "${YELLOW}4. Apri l'Homebrew Launcher${NC}"
        echo -e "${YELLOW}5. Premi A per eseguire l'app${NC}"
        echo -e "${GREEN}╚════════════════════════════════════════╝${NC}"
    else
        echo -e "${RED}❌ Errore nella creazione del 3DSX${NC}"
        exit 1
    fi
else
    echo -e "${RED}❌ 3dsxtool non disponibile${NC}"
    echo -e "${YELLOW}⚠️  Puoi comunque usare il file binario: build/app.bin${NC}"
    echo -e "${YELLOW}Ma per il 3DS serve un wrapper 3DSX o CIA${NC}"
fi

echo -e "${GREEN}✅ Done!${NC}"


# Verifica che l'ELF esista
if [ ! -f "$ELF_FILE" ]; then
    echo -e "${RED}❌ File ELF non trovato: $ELF_FILE${NC}"
    exit 1
fi

echo -e "${GREEN}✅ ELF trovato: $ELF_FILE${NC}"

# Crea file dummy per icon e banner se non esistono
if [ ! -f "$ICON_FILE" ]; then
    echo -e "${YELLOW}📝 Creando file icon dummy...${NC}"
    dd if=/dev/zero of="$ICON_FILE" bs=1 count=512 2>/dev/null
fi

if [ ! -f "$BANNER_FILE" ]; then
    echo -e "${YELLOW}📝 Creando file banner dummy...${NC}"
    dd if=/dev/zero of="$BANNER_FILE" bs=1 count=2048 2>/dev/null
fi

echo -e "${YELLOW}📦 Convertendo ELF a 3DSX...${NC}"

# usa bannertool se disponibile, altrimenti crea 3DSX direttamente
if command -v bannertool &> /dev/null; then
    echo -e "${YELLOW}Usando bannertool per migliore compatibilità...${NC}"
    bannertool makebanner -i "$ICON_FILE" -a "$BANNER_FILE" -o build/banner.bin
fi

# Converti a 3DSX usando arm-none-eabi-objcopy
echo -e "${YELLOW}Estraendo il codice binario dall'ELF...${NC}"
arm-none-eabi-objcopy -O binary "$ELF_FILE" build/app.bin

# Crea header 3DSX minimo
# Formato 3DSX semplificato
echo -e "${YELLOW}Creando file 3DSX...${NC}"

# Scarica 3dsxtool se disponibile, altrimenti mostriamo come procedere
if ! command -v 3dsxtool &> /dev/null; then
    echo -e "${YELLOW}⚠️ 3dsxtool non trovato. Scaricandolo...${NC}"
    
    # Scarica 3dsxtool
    curl -L -o /tmp/3dsxtool.zip "https://github.com/devkitPro/3dsxtool/releases/download/v1.2/3dsxtool" 2>/dev/null || \
    curl -L -o /tmp/3dsxtool "https://github.com/devkitPro/3dsxtool/releases/download/v1.2/3dsxtool-1.2-macos" 2>/dev/null
    
    if [ -f /tmp/3dsxtool ]; then
        sudo mv /tmp/3dsxtool /usr/local/bin/
        sudo chmod +x /usr/local/bin/3dsxtool
    fi
fi

# Crea 3DSX
if command -v 3dsxtool &> /dev/null; then
    echo -e "${YELLOW}Creando 3DSX...${NC}"
    3dsxtool "$ELF_FILE" "$OUTPUT_3DSX" -i "$ICON_FILE" 2>&1
    
    if [ -f "$OUTPUT_3DSX" ]; then
        echo -e "${GREEN}✅ File 3DSX creato con successo!${NC}"
        echo -e "${GREEN}📁 Posizione: $OUTPUT_3DSX${NC}"
        ls -lh "$OUTPUT_3DSX"
        echo ""
        echo -e "${GREEN}╔════════════════════════════════════════╗${NC}"
        echo -e "${GREEN}║  ISTRUZIONI PER 3DS SBLOCCATO (CFW)   ║${NC}"
        echo -e "${GREEN}╠════════════════════════════════════════╣${NC}"
        echo -e "${YELLOW}1. Connetti la SD card al computer${NC}"
        echo -e "${YELLOW}2. Copia: $OUTPUT_3DSX${NC}"
        echo -e "${YELLOW}   In: /3ds/app.3dsx${NC}"
        echo -e "${YELLOW}3. Inserisci la SD card nel 3DS${NC}"
        echo -e "${YELLOW}4. Apri l'Homebrew Launcher${NC}"
        echo -e "${YELLOW}5. Premi A per eseguire l'app${NC}"
        echo -e "${GREEN}╚════════════════════════════════════════╝${NC}"
    else
        echo -e "${RED}❌ Errore nella creazione del 3DSX${NC}"
        exit 1
    fi
else
    echo -e "${RED}❌ 3dsxtool non disponibile${NC}"
    echo -e "${YELLOW}⚠️  Puoi comunque usare il file binario: build/app.bin${NC}"
    echo -e "${YELLOW}Ma per il 3DS serve un wrapper 3DSX o CIA${NC}"
fi

echo -e "${GREEN}✅ Done!${NC}"
