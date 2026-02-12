# Building CIA (Installable Application) Format

## Current Status

The project builds as **`.3dsx` (Homebrew)** by default, which is the easiest format to use.

If you want to create a **`.cia`** (Installable/System Application) format instead, here are your options:

---

## Option 1: Online Converter (Easiest)

The simplest way is to use an online `.3dsx` to `.cia` converter:

1. Build the homebrew: `make all`
2. Upload `x18mixer.3dsx` to https://3dskit.xyz/converter
3. Download the generated `.cia`
4. Transfer to your 3DS via FBI or JKSM

**Pros:** No local tools needed, works reliably
**Cons:** Requires internet, file uploaded to third-party service

---

## Option 2: Local Build with makerom

To build CIA locally on macOS/Linux:

### Requirements
```bash
# You should already have from devkitARM:
which makerom  # Should return devkitARM/tools/bin/makerom
```

### Process

The `.cia` format requires detailed access control configuration. If you want to build locally:

1. Edit `build.rsf` with proper AccessControl sections
2. Run: `makerom -f cia -o x18mixer.cia -elf x18mixer.elf -rsf build.rsf -banner gfx/icon.smdh -icon gfx/icon.smdh -romfs gfx`

**Note:** This is complex. Recommended only if you're familiar with 3DS development.

---

## Installation

To install `.cia` on your New 3DS XL with CFW:

1. **Using FBI:**
   - Launch FBI from Homebrew Menu
   - Navigate to the CIA file
   - Press (A) → Install and confirm

2. **Using JKSM:**
   - Copy CIA to SD card `/` or `/gm9/out/`
   - Launch JKSM
   - Install via game installation menu

---

## When to Use Which Format

| Format | Use Case | Installation |
|--------|----------|--------------|
| `.3dsx` | Default | Homebrew Menu (easiest) |
| `.cia` | Polished / System Integration | FBI/JKSM (more official-looking) |

---

## Recommendation

**For most users:** Use the default `.3dsx` homebrew build. It's simpler and works perfectly on any New 3DS with CFW.

**If you want CIA:** Use the online converter for simplicity, or build locally if familiar with makerom.
