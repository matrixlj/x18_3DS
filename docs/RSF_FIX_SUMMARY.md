# RSF Fix Summary for makerom v0.19.0

## Problem
Your build_spec.rsf was causing this error:
```
[EXHEADER ERROR] Parameter Not Found: "AccessControlInfo/CoreVersion"
```

## Root Cause
The error message was misleading. The real issue was that your RSF file was missing **multiple required parameters** in the `AccessControlInfo` section that are mandatory for makerom v0.19.0:

1. ✗ `DescVersion` - MISSING
2. ✗ `ReleaseKernelMajor` - MISSING
3. ✗ `ReleaseKernelMinor` - MISSING
4. ✗ `MemoryType` - MISSING
5. ✗ `SystemMode` - MISSING

Additionally, your file used an outdated INI-style format instead of YAML, which makerom v0.19.0 requires.

## Solution Applied
Updated [build_spec.rsf](build_spec.rsf) with:

### ✅ Added Required Parameters (Mandatory)
```yaml
AccessControlInfo:
  CoreVersion             : 2              # Firmware core version
  DescVersion             : 2              # ExHeader format version
  ReleaseKernelMajor      : "02"           # Kernel major version
  ReleaseKernelMinor      : "33"           # Kernel minor version (= 4.5.0)
  MemoryType              : Application    # RAM type setting
  SystemMode              : 64MB           # Memory allocation mode
```

### ✅ Added Important ARM11 Settings
```yaml
  IdealProcessor          : 0
  AffinityMask            : 1  
  Priority                : 16
  MaxCpu                  : 0
  HandleTableSize         : 0x200
  EnableL2Cache           : true
  CanAccessCore2          : true
```

### ✅ Added Service & Syscall Access Control
- Full SystemCallAccess list (ARM11 syscalls 0-61 + backdoor)
- Complete ServiceAccessControl list (34 services)
- FileSystemAccess permissions
- IO/Memory mapping

### ✅ Converted Format
Changed from INI-style:
```ini
[AccessControlInfo]
CoreVersion = 2
```

To proper YAML:
```yaml
AccessControlInfo:
  CoreVersion             : 2
  DescVersion             : 2
```

## Testing the New RSF

To verify your updated RSF works with makerom v0.19.0:

```bash
# Method 1: Using -desc (simplest - uses built-in template)
makerom -f cia -o app.cia -rsf build_spec.rsf -target t \
  -elf main.elf -icon icon.bin -banner banner.bnr -desc app:4

# Method 2: Using full RSF (now that it has all required fields)
makerom -f cia -o app.cia -rsf build_spec.rsf -target t \
  -elf main.elf -icon icon.bin -banner banner.bnr
```

## Files Modified
- **[build_spec.rsf](build_spec.rsf)** - Updated with complete RSF v0.19.0 format
- **[RSF_FORMAT_GUIDE.md](RSF_FORMAT_GUIDE.md)** - Complete reference for all parameters

## References
- Official: 3DSGuy/Project_CTR makerom v0.19.0
- Source: https://github.com/3DSGuy/Project_CTR/blob/master/makerom/src/exheader.c
- Compiled working example: https://gist.github.com/jakcron/9f9f02ffd94d98a72632
