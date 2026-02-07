# 🎮 Running X18 Nintendo DS on Emulator

## Quick Start

The application is ready to run on a 3DS emulator or hardware. Here are the available options:

### 1. **Citra Emulator (Recommended)**

If you have [Citra](https://citra-emu.org/) installed:

```bash
make test-emulator
```

This will:
- Compile the application
- Generate the 3DSX file
- Launch Citra with the app (if installed)
- Or fall back to the Python simulator

### 2. **Python Simulator** (No Citra Required)

Run the simulator locally without needing Citra:

```bash
python3 citra_simulator.py build/app.3dsx
```

This provides:
- System initialization output
- CPU/GPU simulation
- Memory information
- Visual framebuffer preview

### 3. **Full Test Suite**

```bash
make test
```

This will:
- Compile the code
- Generate 3DSX file
- Validate 3DSX format
- Show framebuffer preview (`build/framebuffer_preview.png`)

### 4. **Hardware Deployment**

To deploy to actual 3DS hardware with SD card:

```bash
make test-hardware
```

Requires: SD card mounted at `/3ds/` with Homebrew Launcher installed

---

## Build Outputs

- `build/app.3dsx` - Nintendo 3DS Homebrew format (118 KB)
- `build/nds_app.elf` - Raw ARM9 executable
- `build/framebuffer_preview.png` - Visual preview of screen output

## System Requirements

### Minimum (Simulator)
- Python 3.8+
- arm-none-eabi-gcc (already in project)

### For Citra Emulation
- [Citra Emulator](https://citra-emu.org/) installed
- 2GB RAM available

### For Hardware Testing
- Nintendo 3DS with Homebrew Launcher
- SD card (8GB+ recommended)
- USB SD card reader

---

## What Gets Simulated

The Python simulator shows:

### System Initialization
```
ARM9 CPU        ✓ OK
ARM7 CPU        ✓ OK
GPU (PICA200)   ✓ OK
Memory          ✓ OK
I/O Registers   ✓ OK
Framebuffer     ✓ OK
```

### Application Execution
- Entry point: `0x08000000`
- Stack: `0x0B000000`
- Simulated frame rendering (3 frames)
- CPU/GPU usage estimates
- Memory allocation

### Output
- Top screen (400×240) visualization
- Bottom screen (320×240) - touchscreen
- Performance metrics
- Execution success confirmation

---

## Troubleshooting

### "Citra not found"
This is expected if you haven't installed Citra. The simulator will run instead.

### "3DSX file not found"
Run `make clean && make 3dsx` first to compile and generate the binary.

### "Python: command not found"
Install Python 3.8+ or use `python3` instead of `python`.

### Build errors
Check that `arm-none-eabi-gcc` is installed:
```bash
which arm-none-eabi-gcc
arm-none-eabi-gcc --version
```

---

## Project Status

✅ **Phase 8 Complete**: 5-band EQ with integration & testing (118 KB)

All 8 development phases are implemented:
1. ✅ Structure & State Machine
2. ✅ Database & Menu
3. ✅ Mixer Screen (16 channels)
4. ✅ Step List Navigation
5. ✅ OSC Client
6. ✅ Touch Input System
7. ✅ 5-Band EQ Window
8. ✅ Integration & Testing

---

## Next Steps

1. **Test in Simulator**: `make test-emulator`
2. **Verify Parser**: `make test`
3. **Check Framebuffer**: Open `build/framebuffer_preview.png`
4. **Deploy to Hardware** (optional): `make test-hardware`

For real testing with a Behringer X18 mixer, transfer `build/app.3dsx` to your 3DS and connect via OSC.

---

**Last Updated**: February 5, 2026  
**Compiler**: arm-none-eabi-gcc v15.2.0  
**Status**: ✅ Ready for emulator and hardware testing
