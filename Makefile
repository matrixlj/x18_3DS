include $(DEVKITARM)/3ds_rules

TARGET = x18mixer
BUILD = build
SOURCES = src
ICON = gfx/icon.png
ROMFS_DIR := gfx

# Application metadata
APP_TITLE = X18 Mixer
APP_DESCRIPTION = X18 Mixer Controller
APP_AUTHOR = MLJ
APP_VERSION = 1.0.0

ARCH = -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS = -g -Wall -O2 -mword-relocations -ffunction-sections $(ARCH) -D__3DS__
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11

LDFLAGS = -specs=3dsx.specs -g $(ARCH)

LIBS = -lcitro2d -lcitro3d -lctru -lm

CFILES = $(shell find $(SOURCES) -name "*.c")
OFILES = $(addprefix $(BUILD)/, $(CFILES:.c=.o))
DEPENDS = $(OFILES:.o=.d)

CPPFLAGS = $(foreach dir,$(CTRULIB),-I$(dir)/include)

$(TARGET).elf: $(OFILES)
	@echo "🔗 Linking..."
	@$(CC) $(LDFLAGS) $(OFILES) -L$(CTRULIB)/lib $(LIBS) -o $@
	@echo "   Size: $$(ls -lh $@ | awk '{print $$5}')"

$(TARGET).3dsx: $(TARGET).elf
	@echo "📦 Creating 3DSX with metadata..."
	@echo "   Command: 3dsxtool $< $@ --smdh=gfx/icon.smdh --romfs=$(ROMFS_DIR)"
	@3dsxtool $< $@ --smdh=gfx/icon.smdh --romfs=$(ROMFS_DIR)
	@echo "✅ Built: $(TARGET).3dsx"
	@ls -lh $(TARGET).3dsx

$(TARGET).cia: $(TARGET).elf
	@echo "📦 Creating CIA (Installable Application)..."
	@echo "   Command: makerom -f cia -o $@ -target t -elf $< -icon gfx/icon.smdh -desc app:4 -rsf build.rsf"
	@makerom -f cia -o $@ -target t -elf $< -icon gfx/icon.smdh -desc app:4 -rsf build.rsf > /dev/null 2>&1
	@echo "✅ Built: $(TARGET).cia"
	@ls -lh $(TARGET).cia

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "📝 $(notdir $<)"
	@$(CC) -MMD -MP $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: all clean

all: $(TARGET).3dsx

clean:
	@echo "🧹 Cleaning..."
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).3dsx
	@echo "✅ Clean complete"

-include $(DEPENDS)



