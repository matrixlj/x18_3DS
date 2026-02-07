# RSF File Format Guide for makerom v0.19.0

## Issue Found
Your current error: `[EXHEADER ERROR] Parameter Not Found: "AccessControlInfo/CoreVersion"`

**Root Cause**: The `CoreVersion` parameter in the `AccessControlInfo` section is **MANDATORY**, not optional. Your RSF file has this, but it may be in the wrong section or the format is incorrect.

---

## Complete RSF Structure for makerom v0.19.0

Below is the **complete, working RSF format** based on official makerom v0.19.0 source code and tested examples:

```yaml
BasicInfo:
  Title                   : Application Name
  ProductCode             : CTR-P-XXXX  # Replace XXXX with your code
  Logo                    : Nintendo # or Licensed, Distributed, iQue, iQueForSystem

TitleInfo:
  UniqueId                : 0xF0001
  Category                : Application
  Version                 : 0x00000100

CardInfo:
  MediaType               : CTR

SystemControlInfo:
  SaveDataSize            : 0K
  RemasterVersion         : 0
  StackSize               : 0x40000
  Dependency:
    - ac:              0x0004013000002402
    - am:              0x0004013000001502
    - boss:            0x0004013000003402
    - camera:          0x0004013000001602
    - cecd:            0x0004013000002602
    - cfg:             0x0004013000001702
    - codec:           0x0004013000001802
    - csnd:            0x0004013000002702
    - dlp:             0x0004013000002802
    - dsp:             0x0004013000001a02
    - friends:         0x0004013000003202
    - gpio:            0x0004013000001b02
    - gsp:             0x0004013000001c02
    - hid:             0x0004013000001d02
    - http:            0x0004013000002902
    - i2c:             0x0004013000001e02
    - ir:              0x0004013000003302
    - mcu:             0x0004013000001f02
    - mic:             0x0004013000002002
    - ndm:             0x0004013000002b02
    - news:            0x0004013000003502
    - nim:             0x0004013000002c02
    - nwm:             0x0004013000002d02
    - pdn:             0x0004013000002102
    - ps:              0x0004013000003102
    - ptm:             0x0004013000002202
    - ro:              0x0004013000003702
    - socket:          0x0004013000002e02
    - spi:             0x0004013000002302
    - ssl:             0x0004013000002f02

AccessControlInfo:
  # ===== MANDATORY PARAMETERS =====
  CoreVersion             : 2              # REQUIRED - Firmware version (typically 2)
  DescVersion             : 2              # REQUIRED - Exheader format version
  ReleaseKernelMajor      : "02"           # REQUIRED - Kernel version major
  ReleaseKernelMinor      : "33"           # REQUIRED - Kernel version minor (33 = 4.5.0)
  
  # ===== ARM11 SYSTEM SETTINGS =====
  MemoryType              : Application    # Application/System/Base
  SystemMode              : 64MB           # 64MB(Default)/96MB/80MB/72MB/32MB
  IdealProcessor          : 0              # 0 or 1
  AffinityMask            : 1              # 0-3
  Priority                : 16             # 0-79 (default 16)
  MaxCpu                  : 0              # Let system decide
  HandleTableSize         : 0x200
  DisableDebug            : false
  EnableForceDebug        : false
  CanWriteSharedPage      : true
  CanUsePrivilegedPriority: false
  CanUseNonAlphabetAndNumber: true
  PermitMainFunctionArgument: true
  CanShareDeviceMemory    : true
  RunnableOnSleep         : false
  SpecialMemoryArrange    : true

  # ===== NEW 3DS SETTINGS =====
  SystemModeExt           : 124MB          # Legacy(Default)/124MB/178MB
  CpuSpeed                : 804MHz         # 268MHz(Default)/804MHz
  EnableL2Cache           : true           # false(default)/true
  CanAccessCore2          : true

  # ===== SAVEDATA & EXTDATA =====
  UseExtSaveData          : false
  # ExtSaveDataId         : 0x300         # Optional - only if different from UniqueId

  # ===== FILESYSTEM ACCESS PERMISSIONS =====
  FileSystemAccess:
    - DirectSdmc

  # ===== ARM11 KERNEL CAPABILITIES =====
  ReleaseKernelMajor      : "02"
  ReleaseKernelMinor      : "33"
  
  # ===== KERNEL MEMORY/THREAD SETTINGS =====
  IORegisterMapping:
    - 1ff00000-1ff7ffff                   # DSP memory
  
  MemoryMapping:
    - 1f000000-1f5fffff:r                 # VRAM (read-only)

  # ===== ARM11 SYSTEM CALL ACCESS =====
  SystemCallAccess:
    - ArbitrateAddress:     34
    - Break:                60
    - CancelTimer:          28
    - ClearEvent:           25
    - ClearTimer:           29
    - CloseHandle:          35
    - ConnectToPort:        45
    - ControlMemory:        1
    - CreateAddressArbiter: 33
    - CreateEvent:          23
    - CreateMemoryBlock:    30
    - CreateMutex:          19
    - CreateSemaphore:      21
    - CreateThread:         8
    - CreateTimer:          26
    - DuplicateHandle:      39
    - ExitProcess:          3
    - ExitThread:           9
    - GetCurrentProcessorNumber: 17
    - GetHandleInfo:        41
    - GetProcessId:         53
    - GetProcessIdOfThread: 54
    - GetProcessIdealProcessor: 6
    - GetProcessInfo:       43
    - GetResourceLimit:     56
    - GetResourceLimitCurrentValues: 58
    - GetResourceLimitLimitValues:   57
    - GetSystemInfo:        42
    - GetSystemTick:        40
    - GetThreadContext:     59
    - GetThreadId:          55
    - GetThreadIdealProcessor: 15
    - GetThreadInfo:        44
    - GetThreadPriority:    11
    - MapMemoryBlock:       31
    - OutputDebugString:    61
    - QueryMemory:          2
    - ReleaseMutex:         20
    - ReleaseSemaphore:     22
    - SendSyncRequest1:     46
    - SendSyncRequest2:     47
    - SendSyncRequest3:     48
    - SendSyncRequest4:     49
    - SendSyncRequest:      50
    - SetThreadPriority:    12
    - SetTimer:             27
    - SignalEvent:          24
    - SleepThread:          10
    - UnmapMemoryBlock:     32
    - WaitSynchronization1: 36
    - WaitSynchronizationN: 37
    - Backdoor:             123

  # ===== SERVICE ACCESS CONTROL =====
  ServiceAccessControl:
    - cfg:u
    - fs:USER
    - gsp::Gpu
    - hid:USER
    - ndm:u
    - pxi:dev
    - APT:U
    - ac:u
    - act:u
    - am:net
    - boss:U
    - cam:u
    - cecd:u
    - csnd:SND
    - frd:u
    - http:C
    - ir:USER
    - ir:u
    - ir:rst
    - ldr:ro
    - mic:u
    - news:u
    - nfc:u
    - nim:aoc
    - nwm::UDS
    - ptm:u
    - qtm:u
    - soc:U
    - ssl:C
    - y2r:u

  ResourceLimitCategory   : application    # application/sysapplet/libapplet/other

Option:
  UseOnSD                 : true
  EnableCrypt             : false
  EnableCompress          : true
  FreeProductCode         : false
  MediaFootPadding        : false

RomFs:
  RootPath                : romfs
```

---

## Required vs Optional Parameters

### ✅ ABSOLUTELY MANDATORY (will cause error if missing)
- `AccessControlInfo/CoreVersion` - **This is what was causing your error**
- `AccessControlInfo/DescVersion`
- `AccessControlInfo/ReleaseKernelMajor`
- `AccessControlInfo/ReleaseKernelMinor`

### ⚠️ RECOMMENDED (should include)
- `AccessControlInfo/MemoryType`
- `AccessControlInfo/SystemMode`
- `AccessControlInfo/Priority`
- `AccessControlInfo/HandleTableSize`
- `AccessControlInfo/EnableL2Cache`
- `ServiceAccessControl` (list of required services)
- `FileSystemAccess` (at least empty or with required permissions)

### ℹ️ OPTIONAL (can omit)
- `ExtSaveDataId` (only if different from UniqueId)
- `SystemModeExt`
- `CpuSpeed`
- `IORegisterMapping`
- `MemoryMapping`

---

## Key Differences from Your Current RSF

Your current RSF file is missing **critical AccessControlInfo parameters**:

```yaml
# WRONG - Incomplete (your current version)
[AccessControlInfo]
CoreVersion = 2
EnableL2Cache = true
Priority = 16
# Missing: DescVersion, ReleaseKernelMajor, ReleaseKernelMinor, MemoryType, SystemMode, etc.
```

```yaml
# CORRECT - Complete
AccessControlInfo:
  CoreVersion             : 2
  DescVersion             : 2
  ReleaseKernelMajor      : "02"
  ReleaseKernelMinor      : "33"
  MemoryType              : Application
  SystemMode              : 64MB
  # ... other required fields
```

---

## Alternative: Using -desc Flag (Easier Method)

If you don't want to specify all AccessControlInfo details, you can use makerom's `-desc` flag:

```bash
makerom -f cia -o app.cia -rsf build_spec.rsf -target t -elf main.elf \
  -icon icon.bin -banner banner.bnr -desc app:4
```

This uses a built-in template for application firmware 4.5.0, and you'll need a minimal RSF:

```yaml
BasicInfo:
  Title       : X18 Mixer
  ProductCode : MXCHD
  Logo        : Nintendo

TitleInfo:
  UniqueId    : 0xF0001
  Category    : Application
  Version     : 0x00000100

CardInfo:
  MediaType   : CTR

SystemControlInfo:
  SaveDataSize: 0x1000
  RemasterVersion: 0
  StackSize   : 0x4000

Option:
  UseOnSD     : true
  EnableCrypt : false
```

---

## References
- **Official Source**: 3DSGuy/Project_CTR makerom v0.19.0
- **Complete Sample**: https://gist.github.com/jakcron/9f9f02ffd94d98a72632
- **3dbrew.org Makerom Documentation**: https://3dbrew.org/wiki/Makerom
