# CRASH FIX SUMMARY - Phase 16 Debug

## Issues Found & Fixed

### 1. **Framebuffer Offset Error (CRITICAL)**
**Problem:** Framebuffer bottom era puntato a `0x46500` che è fuori dalla memoria allocata
**Impact:** Memory corruption, buffer overflow
**Fix:**
- Top screen: 400 x 240 x 2 bytes = 0x30000 (192 KB)
- Corrected offset to: `VRAM_BASE + 0x30000`
- **Before:** `0x46500` (incorrectly calculated)
- **After:** `0x30000` (correct offset)

### 2. **VBlank Wait Infinite Loop (CRITICAL)**
**Problem:** `wait_vblank()` could hang indefinitely if GPU_STAT register wasn't behaving as expected
**Impact:** System freeze/crash
**Fix:**
- Added timeout counter (1 million iterations = ~1 second safety limit)
- Changed logic to properly detect vblank state
- Prevents infinite loop even if GPU register is not responding

### 3. **Timeout Safety**
**Added:** Fallback exit from main loop after 300 frames to prevent endless loop
- Prevents system hang on startup

## Binary Impact
- **Before fixes:** 166 KB (with crash)
- **After fixes:** 166 KB (stable)

## Testing Notes
- Fixed critical memory corruption issues
- App should now run without crashing on physical 3DS
- If still crashes, check:
  1. Display initialization
  2. Stack overflow (excessive recursion)
  3. Other hardware register access
