# DATA ABORT ROOT CAUSE ANALYSIS

## Problem
App causes **data abort exception** on physical 3DS when trying to access framebuffer.

## Root Cause
Direct memory access to `VRAM_BASE` (0x18000000) is not allowed/mapped on real 3DS hardware.
The issue appears when calling `simple_memset_fb()` to clear screens.

## Failed Attempts
1. ✅ Fixed framebuffer offset (0x46500 → 0x30000) - didn't resolve issue
2. ✅ Added vblank timeout - didn't resolve issue  
3. ✅ Disabled LCD_POWERCNT register - didn't resolve issue
4. ✅ Removed floating point operations - didn't resolve issue

## Root Issue Identification
Through systematic testing, isolated the crash to:
- **render_frame()** → `simple_memset_fb()` → Direct memory write to VRAM

This indicates the 3DS memory map is different or VRAM must be accessed through libctru APIs.

## Solution Path
Use libctru's framebuffer access functions instead of raw pointers:
```c
// Instead of:
unsigned short *fb_top = (unsigned short *)VRAM_BASE;
fb_top[x + y * width] = color;

// Use libctru:
u32* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
((u16*)fb)[x + y * width] = color;
```

## Current Status
- ✅ App initializes successfully (state, managers, OSC, touch)
- ✅ No crash on init
- ❌ Crashes when accessing VRAM framebuffer directly
- ⚠️ Framebuffer access **MUST use libctru APIs**

## Recommended Action
Replace all direct VRAM access with libctru gfxDrawPixel/gfxFillScreen APIs
or implement proper framebuffer mapping using libctru functions.
