# Testing Your 3DS App

Hai 3 opzioni per testare la tua app:

## 🧪 **Opzione 1: Test Parser (Veloce - No Emulatore)**

Testa il 3DSX senza avere un emulatore vero:

```bash
make test
```

Genera:
- Analisi del file 3DSX
- Preview del framebuffer (PNG)
- Disassembly del codice

✅ **Vantaggi**: Nessuna dipendenza esterna, veloce
❌ **Limitazioni**: Non esegue il codice realmente

---

## 🎮 **Opzione 2: Citra Emulator (Se installato)**

Se hai Citra installato:

```bash
make test-emulator
```

Lancia Citra direttamente con il tuo 3DSX.

### Installare Citra (Deprecato ma funzionante)

Homebrew:
```bash
brew install citra
```

Oppure scarica manualmente da:
- https://github.com/citra-emu/citra/releases
- Scarica: `citra-osx-universal.dmg`
- Installa in `/Applications/Citra.app`

✅ **Vantaggi**: Emulazione completa del 3DS
❌ **Limitazioni**: Progetto deprecated (non più mantenuto ufficialmente)

---

## 📱 **Opzione 3: 3DS Hardware (Il Tuo 3DS Fisico)**

Testa direttamente sul tuo 3DS sbloccato:

```bash
# 1. Connetti la SD card al computer
diskutil list              # Identifica il disco
diskutil mount diskXsY     # Monta la partizione

# 2. Deploy
make test-hardware

# 3. Sul 3DS
# - Accendi la console
# - Apri Homebrew Launcher
# - Avvia l'app "app"
```

✅ **Vantaggi**: Test reale, hardware autentico
❌ **Limitazioni**: Richiede il 3DS fisico e SD card

---

## 📊 Confronto delle Opzioni

| Aspetto | Parser | Emulatore | Hardware |
|---------|--------|-----------|----------|
| **Velocità** | ⚡⚡⚡ | ⚡⚡ | ⚡ |
| **Completezza** | 🔴 | 🟢 | 🟢 |
| **Dipendenze** | Python | Citra | 3DS |
| **Accuratezza** | 🔴 | 🟡 | 🟢 |
| **Tempo Setup** | 0 min | 5 min | 10 min |

---

## 🔍 Analisi dei Risultati

### Con `make test`

Vedrai:
```
3DSX File Information:
─ Magic: 3DSX ✅
─ Code Section Size
─ Text Address
─ Opcode preview
```

Se tutto è ✅ il file è valido.

### Con Framebuffer Preview

Il file `build/framebuffer_preview.png` mostra:
- Come l'app disegnerebbe sullo schermo
- Rettangoli colorati (blu, verde, rosso)
- Layout 400x240 (risoluzione 3DS)

---

## 💡 Consigli

1. **Inizia con**: `make test` - rapido e gratuito
2. **Poi prova**: Emulatore se disponibile
3. **Infine testa su**: 3DS fisico per confermare

---

## 🐛 Troubleshooting

### "Citra non trovato"
Fai `make test-emulator` - lo script guide all'installazione

### "SD card non montata"
```bash
diskutil list              # Identifica disco
diskutil mount diskXsY     # Monta
```

### App non appare dopo deploy
- Verifica: `/3ds/app.3dsx` sulla SD
- Cartella `/3ds` deve esistere

---

## 📚 Note Tecniche

- **Parser Python**: Legge header 3DSX e estrae info
- **Emulatore Citra**: Deprecato ma still funzionante
- **QEMU**: Installato come alternativa (per ARM raw)
- **Preview**: Simula il framebuffer 400x240 RGB

---

**Status**: ✅ Pronto per Testing!

Scegli uno dei 3 metodi sopra per iniziare.
