# Nintendo 3DS Development

Questo è un progetto di sviluppo per Nintendo 3DS usando toolchain ARM e format 3DSX.

## ✅ Setup Completato

- ✅ arm-none-eabi-gcc (compilatore ARM)
- ✅ arm-none-eabi-binutils 
- ✅ libctru headers (clonato)
- ✅ create_3dsx.py (tool personalizzato)
- ✅ deploy_3ds.sh (deployment automatico)

## 🚀 Build & Deploy

### Compile
```bash
make              # Compila il codice sorgente
```

### Crea 3DSX per Homebrew Launcher
```bash
make 3dsx         # Crea il file app.3dsx
```

### Estrai binary grezzo
```bash
make binary       # Crea app.bin e app.disasm
```

### Deploy su 3DS (con SD card montata)
```bash
make deploy       # Copia automaticamente su /3ds/app.3dsx
```

### Pulisci build
```bash
make clean        # Rimuove cartella build/
```

## 📋 Istruzioni Passo-Passo

### 1️⃣ Compilazione

```bash
make clean && make
```

Output: `build/nds_app.elf` (9 KB)

### 2️⃣ Crea file 3DSX

```bash
make 3dsx
```

Output: `build/app.3dsx` (14 KB)

### 3️⃣ Copia su 3DS

**Opzione A: Via Script (Automatico)**
```bash
# Connetti SD card al computer
make deploy
```

**Opzione B: Manuale**
```bash
# Connetti SD card
# Copia build/app.3dsx in: /3ds/app.3dsx sulla SD
# Inserisci SD nel 3DS
```

### 4️⃣ Esegui su 3DS

1. Accendi il 3DS
2. Apri l'**Homebrew Launcher**
   - Di solito: Menu Home → Browser → exploit
3. Cerca l'app "app" nella lista
4. Premi **A** per eseguirla

## 📁 Struttura Progetto

```
X18_Nintendo_ds/
├── src/
│   └── main.c           # Codice sorgente principale
├── include/             # Header files
├── build/               # Output compilato
│   ├── nds_app.elf      # Executable ARM
│   ├── app.3dsx         # Homebrew Launcher format
│   ├── app.bin          # Binary grezzo
│   └── app.disasm       # Disassembly
├── Makefile             # Build system
├── create_3dsx.py       # Converter ELF→3DSX
├── deploy_3ds.sh        # Script deployment
└── README.md            # Questo file
```

## 🔧 Modifica Codice

Edita [src/main.c](src/main.c) per modificare il programma.

Attualmente:
- Disegna rettangoli colorati sullo schermo
- Aspetta V-blank per sincronizzazione
- Usa accesso diretto alla memoria video (bare-metal)

### Ricompila dopo modifiche:
```bash
make clean && make 3dsx
```

Poi usa `make deploy` per testare.

## 🐛 Troubleshooting

### Il 3DSX non appare in Homebrew Launcher
```bash
# Verifica che il file sia su:
# /3ds/app.3dsx (sulla SD card)
```

### Crash all'avvio
- Controlla il codice in `src/main.c`
- Visualizza `build/app.disasm` per il disassembly
- Potrebbe essere un errore di memoria

### SD card non montata
```bash
diskutil list              # Identifica il disco
diskutil mount diskXsY     # Monta la partizione
```

## 📚 Formato File

- **ELF** (.elf) → Executable con debug info
- **3DSX** (.3dsx) → Formato Homebrew Launcher
- **Binary** (.bin) → Raw bytecode grezzo

## 🎮 3DS Requirements

- ✅ 3DS sbloccato con CFW (Luma3DS, ecc.)
- ✅ Homebrew Launcher installato
- ✅ SD card formattata

## 📖 Referenze

- [3DS DevKit Wiki](https://3dbrew.org/)
- [DevKitPro](https://devkitpro.org/)
- [libctru](https://github.com/devkitPro/libctru)
- [Project_CTR Tools](https://github.com/3DSGuy/Project_CTR)

## ⚠️ Nota Legale

Questo software è inteso solo per scopi educativi su console personali. Assicurati di rispettare i termini di servizio di Nintendo.

---

**Stato**: ✅ Pronto per il testing su 3DS sbloccato

