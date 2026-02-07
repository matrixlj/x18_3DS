# Hybrid libctru-Style Implementation for 3DS Development

## Overview

This approach provides a **portable, dependency-free way** to develop 3DS applications without requiring the full devkitPro SDK installation. It implements essential libctru functions locally while maintaining API compatibility.

---

## Architecture

### Files Structure

```
project/
├── src/
│   ├── crt0.s              # ARM9 entry point (standard)
│   ├── main_hybrid.c       # Hybrid libctru implementation
│   └── link.ld             # Linker script
├── libctru/                # Downloaded but not compiled (headers only)
│   └── libctru/include/    # 3DS header files (reference)
├── Makefile.libctru        # Build configuration
└── create_3dsx.py          # ELF to 3DSX converter
```

### Key Components

**main_hybrid.c** implements:
- Type definitions (u8, u32, Handle, etc.)
- Framebuffer management (gfxInitDefault, gfxGetFramebuffer)
- Input handling (hidScanInput, hidKeysDown)
- Graphics services (gfxFlushBuffers, gfxSwapBuffers, gspWaitForVBlank)
- Application lifecycle (aptMainLoop)

All functions **stub-compatible** with libctru API but implemented locally.

---

## Compilation

### Requirements

- GCC ARM cross-compiler: `arm-none-eabi-gcc`
- ARM binutils
- Python 3 (for 3DSX conversion)

### Build Process

```bash
# Compile ELF (13 KB)
make -f Makefile.libctru clean
make -f Makefile.libctru

# Convert to 3DSX (6 KB, hbmenu-compatible)
python3 create_3dsx.py build_libctru/x18mixer.elf build_libctru/x18mixer.3dsx "App Name"
```

---

## Limitations & Next Steps

### What This Enables
✅ Compile 3DS applications standalone  
✅ No dependency on devkitPro pacman manager  
✅ Portable code structure  
✅ Works with existing 3DSX game loaders  

### What's Stubbed Out
⚠️ Input handling (returns zeros, needs HID shared memory reading)  
⚠️ VRAM framebuffer write (READ-ONLY in 3DSX; needs CIA format for write)  
⚠️ gspWaitForVBlank (simple delay, not real)  
⚠️ No GPU command acceleration (citro3d replacement)  

### For Full Rendering

To enable actual VRAM writes and color output:

**Option A:** Upgrade to CIA format
- Requires proper devkitPro SDK
- Supports GSP Service context (VRAM write access)
- Full system integration

**Option B:** Enhance hybrid with ARM11 syscalls
- Implement direct memory access
- GSP Service IPC calls
- GPU command submission

---

## Porting to New Projects

### Template Usage

1. Copy `src/crt0.s` (ARM entry point)
2. Copy `src/main_hybrid.c` (remove old main)
3. Copy or reference `link.ld` (linker script)
4. Update Makefile.libctru:
   - Change `SRCS_C` source file reference
   - Adjust output filenames if needed

### Quick Start

```bash
# In new project directory
cp /path/to/X18/src/crt0.s ./src/
cp /path/to/X18/src/main_hybrid.c ./src/main.c  # Rename to main.c
cp /path/to/X18/link.ld ./
cp /path/to/X18/Makefile.libctru ./Makefile
cp /path/to/X18/create_3dsx.py ./

make clean && make
```

---

## API Reference

### Framebuffer Functions

```c
void gfxInitDefault(void);
u8* gfxGetFramebuffer(u32 screen, u32 side, u32* width, u32* height);
void gfxFlushBuffers(void);
void gfxSwapBuffers(void);
void gfxSet3D(u32 enable);
void gfxExit(void);
```

### Input Functions

```c
void hidScanInput(void);
u32 hidKeysDown(void);
u32 hidKeysHeld(void);
u32 hidKeysUp(void);
void hidTouchRead(touchPosition* posOut);
void hidCircleRead(circlePosition* posOut);
```

### Application Control

```c
u8 aptMainLoop(void);  // Returns 0 when app should exit
```

### Graphics Services

```c
void gspWaitForVBlank(void);  // Sync to screen refresh
```

### Constants

```c
#define KEY_A, KEY_B, KEY_X, KEY_Y
#define KEY_L, KEY_R, KEY_ZL, KEY_ZR
#define KEY_START, KEY_SELECT
#define KEY_DUP, KEY_DDOWN, KEY_DLEFT, KEY_DRIGHT
#define KEY_CSTICK_UP, KEY_CSTICK_DOWN, KEY_CSTICK_LEFT, KEY_CSTICK_RIGHT
#define KEY_TOUCH
```

---

## Performance Notes

- **ELF size:** ~13 KB
- **3DSX size:** ~6 KB (compressed)
- **Memory:** Minimal overhead; supports stack-based allocation
- **FPS:** Limited by gspWaitForVBlank simple delay (not hardware synced)

---

## Future Improvements

1. **Real VBlank sync:** Use GSP Service IPC for true synchronization
2. **Input via HID:** Read from shared memory (0x1EC46000)
3. **GPU acceleration:** Implement GPU command queue submission
4. **Multiple RAM buffers:** Double/triple buffering via framebuffer swap
5. **Audio support:** Use CSND Service for sound output
6. **Network:** Use SOC Service for TCP/UDP

---

## Debugging

### Check Compilation
```bash
file build_libctru/x18mixer.elf
arm-none-eabi-objdump -h build_libctru/x18mixer.elf
```

### Verify 3DSX
```bash
ls -lh build_libctru/x18mixer.3dsx
file build_libctru/x18mixer.3dsx
```

### On 3DS Console
- Copy `x18mixer.3dsx` to `/3ds/` on SD card
- Launch from hbmenu
- Check for colors changing (red → green → blue cycle)

---

## Integration with libctru

If you later install full devkitPro SDK:

1. Replace `main_hybrid.c` with proper libctru code
2. Link against libctru.a instead of stub functions
3. Include <3ds.h> and related headers
4. Build with `-mtune=arm946e-s -march=armv5te` (compatible)

The ELF format remains identical; only internal implementations change.

---

## References

- 3DBrew: https://www.3dbrew.org
- libctru: https://github.com/devkitPro/libctru
- 3ds-examples: https://github.com/devkitPro/3ds-examples
