# Building CIA (Installable Application) Format

## Build Both Formats

The project now builds **both formats** from a single ELF:

```bash
# Build .3dsx (Homebrew - traditional approach)
make x18mixer.3dsx

# Build .cia (Installable - appears in home menu)
make x18mixer.cia

# Build both
make all
```

### Technical Details

Both formats contain identical compiled code. The difference is packaging:
- **`.3dsx`**: Loaded by Homebrew Menu loader
- **`.cia`**: Installed via FBI/Godmode9, appears as system title

### Build Command

The CIA generation uses this makerom invocation:

```bash
makerom -f cia -o x18mixer.cia -target t -elf x18mixer.elf \
  -icon gfx/icon.smdh -desc app:4 -rsf build.rsf
```

**Key parameters:**
- `-desc app:4` - Uses firmware4 app descriptor template (handles ExHeader automatically)
- `-icon gfx/icon.smdh` - 48x48 RGB565 icon for home menu display
- `-rsf build.rsf` - Minimal configuration (RomFS + metadata)"

---

## Installation on New 3DS XL with CFW

### Prerequisites
- New 3DS/New 3DS XL with Luma3DS or equivalent CFW
- FBI installer (available on Homebrew Menu)
- `x18mixer.cia` file on SD card

### Installing via FBI

1. Copy `x18mixer.cia` to SD card (root `/` or `/cias/`)
2. Launch FBI from Homebrew Menu
3. Navigate to SD card location where CIA is stored
4. Select `x18mixer.cia` and press **(A)**
5. Choose **"Install and delete CIA"**
6. Wait for installation (~5 seconds)
7. **X18 Mixer** now appears in home menu with its icon

### Installing via Godmode9

1. Copy `x18mixer.cia` to SD card root `/`
2. Launch Godmode9 (press (Start) or (Select) during boot)
3. Navigate to `SD:` section
4. Highlight `x18mixer.cia`
5. Press **(R + A)** to mark file
6. Press **(A)** on CIA path and select installation target
7. Confirm and wait for installation
8. Title appears in home menu

### Advantages of CIA Format vs .3dsx

| Aspect | CIA | .3dsx |
|--------|-----|-------|
| **Appearance** | System home menu | Homebrew Menu launcher |
| **Persistence** | Survives reboots ✅ | Lost on reboot ❌ |
| **Installation** | Permanent | Temporary |
| **Professional Feel** | Like official titles | Like app launcher |

---

## Uninstalling

To remove X18 Mixer (CIA):
- Use **System Settings → Data Management → Software** to uninstall
- Or use **Godmode9** to delete from SD card `/Nintendo 3DS/` path
