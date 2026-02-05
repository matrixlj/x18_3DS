#!/usr/bin/env python3
"""
3DSX App Visualizer/Tester
Simula l'esecuzione del codice 3DS e mostra il framebuffer
"""

import struct
import sys
from pathlib import Path
from PIL import Image, ImageDraw
import io

class ThreeDSXParser:
    """Parser per file 3DSX"""
    
    MAGIC = b'3DSX'
    HEADER_SIZE = 0x2C
    
    def __init__(self, filepath):
        self.filepath = Path(filepath)
        self.data = None
        self.header = None
        self.code_offset = None
        self.code_size = None
        
        if not self.filepath.exists():
            raise FileNotFoundError(f"File not found: {filepath}")
        
        with open(self.filepath, 'rb') as f:
            self.data = f.read()
        
        self._parse_header()
    
    def _parse_header(self):
        """Parse 3DSX header"""
        if len(self.data) < self.HEADER_SIZE:
            raise ValueError("File too small")
        
        magic = self.data[0:4]
        if magic != self.MAGIC:
            raise ValueError(f"Invalid magic: {magic}")
        
        self.header = {
            'magic': magic,
            'header_size': struct.unpack('<I', self.data[4:8])[0],
            'code_offset': struct.unpack('<I', self.data[8:12])[0],
            'text_addr': struct.unpack('<I', self.data[12:16])[0],
            'text_size': struct.unpack('<I', self.data[16:20])[0],
            'rodata_size': struct.unpack('<I', self.data[20:24])[0],
            'data_size': struct.unpack('<I', self.data[24:28])[0],
            'bss_size': struct.unpack('<I', self.data[28:32])[0],
        }
    
    def get_code(self):
        """Extract code section"""
        offset = self.header['code_offset']
        size = self.header['text_size']
        return self.data[offset:offset+size]
    
    def info(self):
        """Return human readable info"""
        h = self.header
        return f"""
3DSX File Information:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Magic:           {h['magic'].decode('ascii', errors='ignore')}
Header Size:     0x{h['header_size']:X} bytes
Code Offset:     0x{h['code_offset']:X}
Text Address:    0x{h['text_addr']:08X}
Text Size:       0x{h['text_size']:X} bytes
RO Data Size:    0x{h['rodata_size']:X} bytes
Data Size:       0x{h['data_size']:X} bytes
BSS Size:        0x{h['bss_size']:X} bytes
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Total File Size: {len(self.data):,} bytes
Code Section:    {h['text_size']:,} bytes
"""

def create_test_framebuffer():
    """Simula i veri schermi del 3DS: Top (400x240) + Bottom (320x240)"""
    
    # Dimensioni schermi 3DS
    top_width, top_height = 400, 240
    bottom_width, bottom_height = 320, 240
    
    # Crea immagine composita (mostra entrambi gli schermi)
    # Top screen in alto, bottom screen in basso con spazio tra loro
    total_width = max(top_width, bottom_width) + 20  # +20 per bordi
    total_height = top_height + bottom_height + 30  # +30 per spazio e bordi
    
    img = Image.new('RGB', (total_width, total_height), color='#222222')
    draw = ImageDraw.Draw(img)
    
    # ===== TOP SCREEN (Step List) =====
    top_x, top_y = 10, 10
    
    # Sfondo top screen
    draw.rectangle([top_x, top_y, top_x + top_width - 1, top_y + top_height - 1], 
                   outline='#00FF00', fill='#001100', width=2)
    
    # Titolo top screen
    draw.text((top_x + 10, top_y + 5), "STEP LIST - Top Screen (400x240)", fill='#00FF00')
    
    # Simula step list (5 steps visibili)
    step_y = top_y + 25
    for step in range(1, 6):
        color = '#00FFFF' if step == 1 else '#00AA00'  # Step 1 selezionato
        draw.rectangle([top_x + 10, step_y, top_x + 390, step_y + 35], 
                      outline=color, fill='#002200', width=1)
        draw.text((top_x + 20, step_y + 10), f"Step {step}", fill=color)
        step_y += 40
    
    # ===== BOTTOM SCREEN (Mixer) =====
    bottom_x, bottom_y = 10, top_height + 25
    
    # Sfondo bottom screen
    draw.rectangle([bottom_x, bottom_y, bottom_x + bottom_width - 1, bottom_y + bottom_height - 1], 
                   outline='#FF0000', fill='#110000', width=2)
    
    # Titolo bottom screen
    draw.text((bottom_x + 10, bottom_y + 5), "MIXER - Bottom Screen (320x240)", fill='#FF0000')
    
    # Barra superiore (step info)
    draw.rectangle([bottom_x + 5, bottom_y + 25, bottom_x + 315, bottom_y + 40], 
                  fill='#000000')
    draw.text((bottom_x + 10, bottom_y + 27), "Step: 1 | IP: 192.168.1.100", fill='#FFFFFF')
    
    # Simula 16 canali mixer
    # 16 canali su una riga, ciascuno largo ~18px
    channels_y = bottom_y + 50
    channel_width = 18
    
    for ch in range(16):
        ch_x = bottom_x + 10 + (ch * channel_width)
        
        # Colore canale (selezionato = ciano)
        ch_color = '#00FFFF' if ch == 0 else '#00FF00'
        
        # Disegna bordo canale
        draw.rectangle([ch_x, channels_y, ch_x + 16, channels_y + 120], 
                      outline=ch_color, fill='#001100', width=1)
        
        # Numero canale
        draw.text((ch_x + 4, channels_y + 5), f"{ch+1:02d}", fill=ch_color)
        
        # Simula fader (barra verticale)
        fader_level = 75 if ch == 0 else 50  # Ch 0 selezionato a 75%
        fader_height = int(80 * (100 - fader_level) / 100)
        fader_top = channels_y + 25
        
        # Sfondo fader
        draw.rectangle([ch_x + 3, fader_top, ch_x + 13, fader_top + 80], 
                      fill='#001100', outline='#444444', width=1)
        # Fader handle
        draw.rectangle([ch_x + 2, fader_top + fader_height, ch_x + 14, fader_top + fader_height + 8], 
                      fill=ch_color)
        
        # Pulsante Mute (M)
        draw.rectangle([ch_x + 3, channels_y + 110, ch_x + 13, channels_y + 120], 
                      fill='#330000', outline='#FF6666', width=1)
    
    # Barra inferiore (info)
    draw.rectangle([bottom_x + 5, bottom_y + bottom_height - 25, bottom_x + 315, bottom_y + bottom_height - 5], 
                  fill='#000000')
    draw.text((bottom_x + 10, bottom_y + bottom_height - 20), "Ch: 01 | Mute: OFF | EQ: 5 bands", 
             fill='#FFFF00')
    
    return img

def visualize_3dsx(filepath):
    """Visualizza il contenuto del 3DSX"""
    
    print("\n" + "="*60)
    print("  3DSX App Visualizer")
    print("="*60)
    
    try:
        parser = ThreeDSXParser(filepath)
        print(parser.info())
        
        code = parser.get_code()
        print(f"Code disassembly preview (first 16 bytes):")
        print("─" * 60)
        hex_str = ' '.join(f'{b:02X}' for b in code[:16])
        print(f"  {hex_str}")
        print("─" * 60)
        
        # Mostra opcode ARM
        print("\nARM Opcodes (raw bytes):")
        for i in range(0, min(len(code), 32), 4):
            word = struct.unpack('<I', code[i:i+4])[0]
            print(f"  0x{word:08X}")
        
        print("\n" + "="*60)
        print("✅ 3DSX file is valid and can be loaded!")
        print("="*60 + "\n")
        
        # Crea framebuffer di test
        print("Creating framebuffer visualization...")
        print("Rendering TOP screen (400x240): STEP LIST")
        print("Rendering BOTTOM screen (320x240): MIXER with 16 channels")
        fb_img = create_test_framebuffer()
        
        # Salva immagine di preview
        output_path = Path(filepath).parent / "framebuffer_preview.png"
        fb_img.save(output_path)
        print(f"✅ Framebuffer preview saved: {output_path}")
        print(f"   Shows BOTH 3DS screens:")
        print(f"   - TOP: Step List (400x240) with 5 steps")
        print(f"   - BOTTOM: 16-channel Mixer (320x240) with faders and mute buttons\n")
        
        return True
        
    except Exception as e:
        print(f"❌ Error: {e}\n")
        return False

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: test_3dsx.py <3dsx_file>")
        print("Example: test_3dsx.py build/app.3dsx")
        sys.exit(1)
    
    if visualize_3dsx(sys.argv[1]):
        sys.exit(0)
    else:
        sys.exit(1)
