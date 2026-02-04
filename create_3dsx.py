#!/usr/bin/env python3
"""
Simple 3DSX wrapper generator for compiled ELF files
Generates a basic 3DSX file that can be loaded by Homebrew Launcher
"""

import struct
import sys
from pathlib import Path

class SMDH:
    """SMDH (Short Message Data Handler) - App metadata"""
    
    MAGIC = b'SMDH'
    VERSION = 0
    
    def __init__(self, title="Homebrew", desc="A homebrew application", author="Developer"):
        self.data = bytearray(0x36C)  # Fixed size
        
        # Header
        self.data[0:4] = self.MAGIC
        struct.pack_into('<HH', self.data, 4, self.VERSION, 0)
        
        # Settings (flags, ratings, etc.)
        struct.pack_into('<I', self.data, 8, 0)  # Flags
        struct.pack_into('<I', self.data, 12, 0) # Region lock (0 = all)
        
        # Ratings (8 bytes)
        self.data[16:24] = b'\xFF' * 8
        
        # Simple text fields (titles)
        offset = 0x24
        
        # Game title (16 chars max per language, 16 languages)
        for i in range(16):
            text = title.encode('utf-16-le')[:30]
            self.data[offset:offset+32] = text + b'\x00' * (32 - len(text))
            offset += 32
        
        # Short description
        for i in range(16):
            text = desc.encode('utf-16-le')[:60]
            self.data[offset:offset+64] = text + b'\x00' * (64 - len(text))
            offset += 64
        
        # Publisher/Author name
        for i in range(16):
            text = author.encode('utf-16-le')[:30]
            self.data[offset:offset+32] = text + b'\x00' * (32 - len(text))
            offset += 32

class ThreeDSX:
    """3DSX file format wrapper"""
    
    MAGIC = b'3DSX'
    HEADER_SIZE = 0x2C
    
    def __init__(self, code_data, smdh_data=None):
        self.header = bytearray(self.HEADER_SIZE)
        self.code = code_data
        self.smdh = smdh_data if smdh_data else b''
        
        # Magic
        self.header[0:4] = self.MAGIC
        
        # Header size (0x2C = 44 bytes)
        struct.pack_into('<I', self.header, 4, self.HEADER_SIZE)
        
        # Relocation table offset (usually right after header)
        struct.pack_into('<I', self.header, 8, self.HEADER_SIZE)
        
        # Sections info
        # .text at 0x8000
        struct.pack_into('<I', self.header, 0xC, 0x8000)
        struct.pack_into('<I', self.header, 0x10, len(self.code))
        
        # .rodata (empty)
        struct.pack_into('<I', self.header, 0x14, 0)
        struct.pack_into('<I', self.header, 0x18, 0)
        
        # .data (empty)
        struct.pack_into('<I', self.header, 0x1C, 0)
        struct.pack_into('<I', self.header, 0x20, 0)
        
        # .bss (empty)
        struct.pack_into('<I', self.header, 0x24, 0)
        
        # Flags
        struct.pack_into('<I', self.header, 0x28, 0)
    
    def save(self, filename):
        """Write 3DSX file to disk"""
        with open(filename, 'wb') as f:
            f.write(self.header)
            f.write(self.code)
            
            if self.smdh:
                # Alignment padding to next 0x1000
                current_pos = len(self.header) + len(self.code)
                padding_size = (0x1000 - (current_pos % 0x1000)) % 0x1000
                f.write(b'\x00' * padding_size)
                
                # SMDH section
                smdh_header = b'EXAC'  # Extension header
                smdh_header += struct.pack('<I', len(self.smdh))
                f.write(smdh_header)
                f.write(self.smdh)

def create_3dsx(elf_path, output_path, title="Homebrew"):
    """Convert ELF to 3DSX format"""
    
    # Read ELF binary
    elf_file = Path(elf_path)
    if not elf_file.exists():
        print(f"Error: {elf_path} not found")
        return False
    
    try:
        with open(elf_file, 'rb') as f:
            code_data = f.read()
    except Exception as e:
        print(f"Error reading ELF: {e}")
        return False
    
    # Create SMDH metadata
    smdh = SMDH(title=title, desc="A homebrew application", author="Developer")
    
    # Create 3DSX wrapper
    threex = ThreeDSX(code_data, bytes(smdh.data))
    
    try:
        threex.save(output_path)
        print(f"✅ Created: {output_path}")
        size = Path(output_path).stat().st_size
        print(f"   Size: {size:,} bytes ({size/1024:.1f} KB)")
        return True
    except Exception as e:
        print(f"Error writing 3DSX: {e}")
        return False

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: create_3dsx.py <elf_file> <output_3dsx> [title]")
        print("Example: create_3dsx.py build/nds_app.elf build/app.3dsx 'Hello 3DS'")
        sys.exit(1)
    
    elf_path = sys.argv[1]
    output_path = sys.argv[2]
    title = sys.argv[3] if len(sys.argv) > 3 else "Homebrew"
    
    if create_3dsx(elf_path, output_path, title):
        print("\n✅ Conversion successful!")
        print(f"📁 Copy to: /3ds/{Path(output_path).name}")
    else:
        print("❌ Conversion failed")
        sys.exit(1)
