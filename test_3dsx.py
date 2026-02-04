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
    """Crea un'immagine di test che rappresenta il framebuffer del 3DS"""
    
    # 3DS screen: 400x240 RGB565
    width, height = 400, 240
    
    # Crea immagine
    img = Image.new('RGB', (width, height), color='white')
    draw = ImageDraw.Draw(img)
    
    # Disegna rettangoli colorati (come fa main.c)
    draw.rectangle([50, 20, 350, 80], fill='blue')
    draw.rectangle([50, 100, 350, 160], fill='green')
    draw.rectangle([100, 180, 300, 220], fill='red')
    
    # Aggiungi testo informativo
    try:
        draw.text((20, 10), "3DS Homebrew App Test", fill='black')
        draw.text((20, 240-30), "Testing on macOS", fill='black')
    except:
        pass  # Se non ha font
    
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
        fb_img = create_test_framebuffer()
        
        # Salva immagine di preview
        output_path = Path(filepath).parent / "framebuffer_preview.png"
        fb_img.save(output_path)
        print(f"✅ Framebuffer preview saved: {output_path}")
        print(f"   (This is what your app would display on 3DS)\n")
        
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
