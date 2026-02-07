# Troubleshooting Guide: makerom v0.19.0 CIA Build

## Error: [EXHEADER ERROR] Parameter Not Found: "AccessControlInfo/CoreVersion"

### Cause Analysis
This error occurs when makerom v0.19.0 cannot find **one or more required parameters** in the AccessControlInfo section of your RSF file. The error message only reports the first missing parameter, so there may be other missing fields.

### Solution Checklist

✅ **All required parameters are now in your updated RSF**

If you still get this error:

1. **Verify YAML syntax is correct:**
   - Use spaces, not tabs
   - Ensure proper indentation under `AccessControlInfo:`
   - Check for typos in parameter names (case-sensitive)

   ```yaml
   # CORRECT
   AccessControlInfo:
     CoreVersion             : 2
     DescVersion             : 2
   
   # WRONG (tab instead of spaces)
   AccessControlInfo:
   	CoreVersion: 2
   
   # WRONG (incorrect name)
   AccessControlInfo:
     coreversion: 2
   ```

2. **Verify all mandatory fields exist:**
   ```yaml
   AccessControlInfo:
     CoreVersion             : 2              # ✓ Required
     DescVersion             : 2              # ✓ Required
     ReleaseKernelMajor      : "02"           # ✓ Required
     ReleaseKernelMinor      : "33"           # ✓ Required
     MemoryType              : Application    # ✓ Required
     SystemMode              : 64MB           # ✓ Required
   ```

3. **Use correct makerom syntax:**
   ```bash
   # Correct command for building with RSF
   makerom -f cia -o app.cia -rsf build_spec.rsf -target t \
     -elf main.elf -icon icon.bin -banner banner.bnr
   
   # Or use -desc flag (bypasses RSF requirements)
   makerom -f cia -o app.cia -rsf build_spec.rsf -target t \
     -elf main.elf -icon icon.bin -banner banner.bnr -desc app:4
   ```

---

## Other Common Errors & Fixes

### Error: [EXHEADER ERROR] Unexpected SystemMode: ...
**Cause:** Invalid value for SystemMode  
**Fix:** Use one of: `64MB`, `96MB`, `80MB`, `72MB`, `32MB`

### Error: [EXHEADER ERROR] Unexpected AffinityMask: ...
**Cause:** Invalid value for AffinityMask  
**Fix:** Use 0-3 (hex: 0x0-0x3)

### Error: [EXHEADER ERROR] Unexpected IdealProcessor: ...
**Cause:** Invalid value for IdealProcessor  
**Fix:** Use 0 or 1

### Error: [EXHEADER ERROR] Service Name: "..." is too long
**Cause:** Service names must be ≤ 8 characters  
**Fix:** Verify ServiceAccessControl names are 8 characters or less:
```yaml
ServiceAccessControl:
  - cfg:u          # 6 chars ✓
  - fs:USER        # 6 chars ✓
  - gsp::Gpu       # 7 chars ✓
```

### Error: Too Many Service Names, maximum is 34
**Cause:** More than 34 services listed  
**Fix:** Remove unnecessary services from ServiceAccessControl list (keep only needed ones)

### Error: [EXHEADER ERROR] Invalid FileSystemAccess Name: "..."
**Cause:** Typo in FileSystemAccess entry  
**Valid names:**
- CategorySystemApplication
- CategoryHardwareCheck
- CategoryFileSystemTool
- Debug
- TwlCardBackup
- TwlNandData
- Boss
- DirectSdmc
- Core
- CtrNandRo
- CtrNandRw
- CtrNandRoWrite
- CategorySystemSettings
- CardBoard
- ExportImportIvs
- DirectSdmcWrite
- SwitchCleanup
- SaveDataMove
- Shop
- Shell
- CategoryHomeMenu
- SeedDB

---

## Kernel Version Reference

The `ReleaseKernelMinor` value determines firmware compatibility:

| Minor | Firmware | Notes |
|-------|----------|-------|
| 27 | 1.0.46 - 1.2.0 | Old firmware |
| 33 | 4.0.0 - 4.5.0 | **Recommended for homebrew** |
| 44 | 5.0.0 - 5.1.0 | Requires 5.0+ |
| 49 | 9.0.0+ | Latest firmware |

**Recommendation:** Use `33` (4.5.0) for maximum compatibility

---

## SystemMode Memory Reference

| Value | RAM | Use Case |
|-------|-----|----------|
| **64MB** | Default (Old 3DS) | ✓ Most applications |
| 96MB | Extended (Old 3DS) | Graphics-heavy apps |
| 80MB | Extended variant | Rarely used |
| 72MB | Extended variant | Rarely used |
| 32MB | Restricted | System apps only |

**Recommendation:** Use `64MB` for normal applications

---

## Advanced: Custom RSF Without Pre-set

If you need special permissions beyond firmware 4.5.0, create a minimal RSF with only essential fields and use `-desc`:

```bash
makerom -f cia -o app.cia -rsf minimal.rsf -target t \
  -elf main.elf -icon icon.bin -banner banner.bnr -desc app:4
```

Where `minimal.rsf` contains:
```yaml
BasicInfo:
  Title       : App Name
  ProductCode : CTR-P-XXXX
  Logo        : Nintendo

TitleInfo:
  UniqueId    : 0xF0001
  Category    : Application
  Version     : 0x00000100

CardInfo:
  MediaType   : CTR

SystemControlInfo:
  SaveDataSize: 0K
  RemasterVersion: 0
  StackSize   : 0x4000

Option:
  UseOnSD     : true
```

The `-desc app:4` flag fills in the AccessControlInfo automatically.

---

## Validation Checklist Before Building

Before running makerom:

- [ ] RSF file uses YAML format (not INI)
- [ ] All parameters use proper spacing/indentation
- [ ] `AccessControlInfo/CoreVersion` exists and equals 2
- [ ] `AccessControlInfo/DescVersion` exists and equals 2  
- [ ] `AccessControlInfo/ReleaseKernelMajor` = "02"
- [ ] `AccessControlInfo/ReleaseKernelMinor` = "33"
- [ ] `MemoryType` = Application, System, or Base
- [ ] `SystemMode` = one of: 64MB, 96MB, 80MB, 72MB, 32MB
- [ ] `IdealProcessor` = 0 or 1
- [ ] `AffinityMask` = 0-3
- [ ] ELF file exists and path is correct
- [ ] Icon/Banner files exist if specified
- [ ] RomFS enabled if needed

---

## Quick Command Reference

```bash
# Test build with -desc (avoid RSF issues)
makerom -f cia -o test.cia -rsf build_spec.rsf -target t \
  -elf main.elf -icon icon.bin -banner banner.bnr -desc app:4

# Full build with complete RSF
makerom -f cia -o app.cia -rsf build_spec.rsf -target t \
  -elf main.elf -icon icon.bin -banner banner.bnr

# Verbose output for debugging  
makerom -v -f cia -o app.cia -rsf build_spec.rsf -target t \
  -elf main.elf -icon icon.bin -banner banner.bnr -desc app:4

# Convert existing CXI to CIA
makerom -f cia -o app.cia -rsf build_spec.rsf -target t -i app.cxi:0:0
```
