# Nintendo 3DS Development - ELF to CIA Guide

## 📦 Processo di Conversione

### Attualmente abbiamo:
- ✅ **build/nds_app.elf** - Executable ARM32
- ✅ **build/app.bin** - Binary grezzo estratto
- ✅ **build/app.disasm** - Disassembly per debugging

---

## 🚀 Metodi per eseguire sul 3DS

### **OPZIONE 1: Homebrew Launcher (CONSIGLIATA)**

La più semplice e veloce per il testing.

#### Requisiti:
- ✅ 3DS sbloccato con CFW (Luma3DS, ecc.)
- ✅ Homebrew Launcher installato
- ✅ SD card

#### Passaggi:

1. **Scarica 3dsxtool**
   ```bash
   # Visita: https://github.com/devkitPro/3dsxtool/releases
   # Scarica: 3dsxtool-1.x-macos.zip
   ```

2. **Converti ELF a 3DSX**
   ```bash
   3dsxtool build/nds_app.elf build/app.3dsx
   ```

3. **Copia sulla SD card**
   ```
   SD:/3ds/app.3dsx
   ```

4. **Esegui sul 3DS**
   - Apri Homebrew Launcher
   - Premi A su "app"

---

### **OPZIONE 2: CIA Install (Distribuzione)**

Crea un file .CIA per installare l'app come una vera applicazione.

#### Requisiti:
- bannertool (per creare banner)
- makerom (per creare CIA)
- FBI (installer CIA sul 3DS)

#### Passaggi:

```bash
# 1. Scarica i tool
curl -O https://github.com/3DSGuy/Project_CTR/releases/...

# 2. Crea banner
bannertool makebanner -i icon.png -a audio.wav -o banner.bin

# 3. Crea SMDH (metadata dell'app)
bannertool makesmdh -s "Titolo" -l "Descrizione" \
  -p "Autore" -i icon.png -o icon.smdh

# 4. Crea CXI (Content eXecution Image)
makerom -f elf -o app.cxi -rsf spec.rsf \
  -icon icon.smdh -banner banner.bin \
  build/nds_app.elf

# 5. Crea CIA (CTR Importable Archive)
makerom -f cia -o app.cia -content app.cxi:0
```

#### Installa sul 3DS:
```
SD:/cia/app.cia
Apri FBI > Installa CIA > Seleziona app.cia
```

---

### **OPZIONE 3: Direct Boot (Avanzato)**

Alcuni CFW supportano il boot diretto da file binario.

```bash
# Copia il binario nella root della SD
cp build/app.bin /Volumes/3DS/boot.bin

# Riavvia il 3DS
# Il CFW caricherà automaticamente il programma
```

---

## 📊 Confronto dei Metodi

| Aspetto | Homebrew | CIA | Direct Boot |
|---------|----------|-----|-------------|
| **Facilità** | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐ |
| **Testing** | ⭐⭐⭐⭐⭐ | ⭐⭐ | ⭐⭐ |
| **Distribuzione** | ⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐ |
| **Permanenza** | Temporanea | Permanente | Dipende CFW |
| **Velocità Setup** | 2 min | 10 min | 5 min |

---

## 🔧 Tool Necessari

### Homebrew Launcher:
```bash
# 3dsxtool
https://github.com/devkitPro/3dsxtool/releases

# Già istallato:
arm-none-eabi-gcc  ✅
arm-none-eabi-objcopy ✅
```

### CIA Creation:
```bash
# bannertool
https://github.com/3DSGuy/Project_CTR/releases

# makerom  
https://github.com/3DSGuy/Project_CTR/releases  ✅ (già installato)
```

---

## 📝 Formato ELF vs Binary vs CIA

```
┌─────────────────┐
│  main.c (Source)│
└────────┬────────┘
         │ gcc
         ▼
┌─────────────────────────────┐
│   nds_app.elf (ELF)         │  ◄─ Debug info + Metadata
│  - Sections (.text, .data)  │
│  - Symbol table             │
│  - Relocations              │
└────────┬────────────────────┘
         │ objcopy -O binary
         ▼
┌─────────────────────────────┐
│   app.bin (Raw Binary)      │  ◄─ Puro bytecode
│  - Solo codice macchina     │
│  - No metadata              │
└──────────────────────────────┘
         │ 3dsxtool or makerom
         ▼
┌─────────────────────────────┐
│   app.3dsx or app.cia       │  ◄─ Formato eseguibile 3DS
│  - Wrapper + Metadata       │
│  - Icon + Banner            │
│  - Signature                │
└─────────────────────────────┘
         │
         ▼
   [Nintendo 3DS] ▶ Esecuzione
```

---

## 🎯 Prossimi Step

1. **Aggiungi 3dsxtool**
   ```bash
   # Scarica e installa 3dsxtool per creare .3dsx direttamente
   ```

2. **Crea icon/banner**
   ```bash
   # Aggiungi PNG come icona e audio per banner
   ```

3. **Testa sul 3DS fisico**
   ```bash
   # Copia .3dsx su SD e avvia via Homebrew Launcher
   ```

4. **Distribuisci come CIA**
   ```bash
   # Se vuoi distribuire l'app a altri utenti
   ```

---

## 🐛 Debugging

Se il programma non si avvia:

1. **Verifica il formato ELF**
   ```bash
   file build/nds_app.elf
   arm-none-eabi-readelf -h build/nds_app.elf
   ```

2. **Visualizza il codice disassemblato**
   ```bash
   arm-none-eabi-objdump -d build/nds_app.elf | head -50
   ```

3. **Verifica le dipendenze**
   ```bash
   arm-none-eabi-nm build/nds_app.elf | grep "U "
   ```

---

## 📚 Referenze

- [3DS Developer Wiki](https://3dbrew.org/)
- [DevKitPro Docs](https://devkitpro.org/)
- [Project_CTR Tools](https://github.com/3DSGuy/Project_CTR)
- [libctru](https://github.com/devkitPro/libctru)

---

**Status**: ✅ Pronto per il testing su 3DS sbloccato
