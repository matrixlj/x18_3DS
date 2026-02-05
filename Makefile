TARGET = nds_app
SOURCES = src
INCLUDES = include
BUILD = build

# DevKit paths
DEVKITPRO ?= /Users/lorenzomazzocchetti/Desktop/Github/X18_Nintendo_ds/libctru
LIBCTRU_INCLUDE = $(DEVKITPRO)/libctru/include

# Compiler
CC = arm-none-eabi-gcc
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy

# Flags per ARM9 (3DS/DS)
CFLAGS = -march=armv5te -mtune=arm946e-s -Wall -O2 -g -ffreestanding
CFLAGS += -I$(INCLUDES) -I$(LIBCTRU_INCLUDE) -I.
LDFLAGS = -nostdlib

# File sources - now includes subdirectories (exclude tests from binary)
CSOURCES = $(shell find $(SOURCES) -name "*.c" ! -path "*/tests/*")
OBJECTS = $(CSOURCES:%.c=$(BUILD)/%.o)
OUTPUT = $(BUILD)/$(TARGET)

ELF_FILE = $(OUTPUT).elf
3DSX_FILE = $(BUILD)/app.3dsx
BIN_FILE = $(BUILD)/app.bin
DISASM_FILE = $(BUILD)/app.disasm

.PHONY: all clean info 3dsx deploy help

all: $(ELF_FILE)
	@echo "✅ Build complete!"
	@echo "   ELF: $(ELF_FILE)"
	@echo "   Run 'make 3dsx' to create 3DSX for Homebrew Launcher"

$(BUILD):
	mkdir -p $(BUILD) $(BUILD)/core $(BUILD)/screens $(BUILD)/osc $(BUILD)/storage $(BUILD)/ui $(BUILD)/config

$(BUILD)/%.o: %.c | $(BUILD)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF_FILE): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@
	@arm-none-eabi-objdump -h $@

# Converti a 3DSX (Homebrew Launcher format)
3dsx: $(ELF_FILE)
	@echo "📦 Creando 3DSX..."
	python3 create_3dsx.py $(ELF_FILE) $(3DSX_FILE) "Hello 3DS"
	@echo "✅ 3DSX creato: $(3DSX_FILE)"
	@ls -lh $(3DSX_FILE)

# Converti a binary
binary: $(ELF_FILE)
	@echo "📦 Estraendo binary..."
	arm-none-eabi-objcopy -O binary $(ELF_FILE) $(BIN_FILE)
	arm-none-eabi-objdump -d $(ELF_FILE) > $(DISASM_FILE)
	@echo "✅ Binary creato:"
	@ls -lh $(BIN_FILE)

# Deploy su SD card
deploy: 3dsx
	@echo "🎮 Deploying to 3DS..."
	@bash deploy_3ds.sh

# Info
info:
	@echo "DEVKITPRO: $(DEVKITPRO)"
	@echo "Compiler: $(CC)"
	@echo "Flags: $(CFLAGS)"

# Test targets
test: 3dsx
	@echo "🧪 Testing 3DSX..."
	python3 test_3dsx.py $(3DSX_FILE)
	@echo ""
	@echo "✅ Framebuffer preview: build/framebuffer_preview.png"

test-emulator: 3dsx
	@echo "🎮 Launching Citra Emulator..."
	@if command -v citra &> /dev/null || [ -d "/Applications/Citra.app" ]; then \
		bash test_emulator.sh; \
	else \
		echo "⚠️  Citra not found. Using simulator instead..."; \
		python3 citra_simulator.py $(3DSX_FILE); \
	fi

test-hardware: deploy
	@echo "🎮 Deploying to 3DS Hardware..."
	@echo "✅ Done! Check your 3DS Homebrew Launcher"

help:
	@echo ""
	@echo "╔════════════════════════════════════════╗"
	@echo "║     Nintendo 3DS Homebrew Build        ║"
	@echo "╚════════════════════════════════════════╝"
	@echo ""
	@echo "📋 Available targets:"
	@echo ""
	@echo "  make              - Compila l'ELF"
	@echo "  make 3dsx         - Crea 3DSX per Homebrew Launcher"
	@echo "  make binary       - Estrae binary grezzo"
	@echo "  make deploy       - Copia su SD card (necessita mount)"
	@echo "  make clean        - Ripulisce build/"
	@echo ""
	@echo "🧪 Testing targets:"
	@echo ""
	@echo "  make test         - Visualizza il 3DSX (Python parser)"
	@echo "  make test-emulator - Avvia Citra emulator (se installato)"
	@echo "  make test-hardware - Deploy su 3DS fisico (SD card)"
	@echo ""
	@echo "🚀 Workflow tipico:"
	@echo ""
	@echo "  1. make           # Compila"
	@echo "  2. make 3dsx      # Crea 3DSX"
	@echo "  3. make test      # Testa nel parser"
	@echo "  4. make test-emulator  # Testa in emulatore (opz.)"
	@echo "  5. make test-hardware  # Testa su 3DS (opz.)"
	@echo ""

clean:
	rm -rf $(BUILD)

.PHONY: all 3dsx binary deploy clean info help test test-emulator test-hardware test test-emulator test-hardware

