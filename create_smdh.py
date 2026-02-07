#!/usr/bin/env python3
"""
Create a proper SMDH file for 3DS applications.
SMDH format (0x36C0 bytes total):
- 0x0000: Magic "SMDH" (4 bytes)
- 0x0004: Version (2 bytes)
- 0x0006: Reserved (2 bytes)
- 0x0008: 16 Application Title structs (0x2000 bytes total, each 0x200)
- 0x2008: Application Settings (0x30 bytes)
- 0x2038: Reserved (0x8 bytes)
- 0x2040: Small icon 24x24 RGB565 (0x480 bytes)
- 0x24C0: Large icon 48x48 RGB565 (0x1200 bytes)
"""

import struct
from PIL import Image

def morton_interleave(x, y):
    """Create Morton code by interleaving bits of x and y (for 8x8 tiles)"""
    # Only use the lower 3 bits (0-7 range for 8x8)
    x = x & 7
    y = y & 7
    
    result = 0
    for i in range(3):
        result |= ((x >> i) & 1) << (2 * i)
        result |= ((y >> i) & 1) << (2 * i + 1)
    return result

def create_tiled_icon(image, width, height):
    """Convert image to Morton-ordered tiled format (3DS format)
    
    3DS icons use:
    - Tiles of 8x8 pixels
    - Within each tile, Morton order (Z-order curve)
    - Tiles arranged left-to-right, top-to-bottom
    """
    pixels = bytearray()
    
    # Create pixel data array
    pixel_data = {}
    for y in range(height):
        for x in range(width):
            pix = image.getpixel((x, y))
            if hasattr(pix, '__len__'):
                r, g, b = pix[:3]
            else:
                r = g = b = pix
            # Convert RGB888 to RGB565 in little-endian
            rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
            pixel_data[(x, y)] = struct.pack('<H', rgb565)
    
    # Process in 8x8 tiles
    for tile_y in range(0, height, 8):
        for tile_x in range(0, width, 8):
            # Create 64-pixel tile in Morton order
            tile_pixels = [None] * 64
            
            for in_y in range(8):
                for in_x in range(8):
                    x = tile_x + in_x
                    y = tile_y + in_y
                    morton_idx = morton_interleave(in_x, in_y)
                    
                    if x < width and y < height:
                        tile_pixels[morton_idx] = pixel_data[(x, y)]
                    else:
                        tile_pixels[morton_idx] = b'\x00\x00'
            
            # Add tile to output
            for pix in tile_pixels:
                pixels += pix if pix else b'\x00\x00'
    
    return bytes(pixels)

def create_smdh(title, description, author, icon_path, output_path):
    """Create a proper SMDH file"""
    
    # Load and prepare icon
    icon = Image.open(icon_path).convert('RGB')
    
    # Initialize SMDH with correct structure
    smdh = bytearray(0x36C0)  # Total size
    
    # 0x0000: Magic
    smdh[0x0000:0x0004] = b'SMDH'
    
    # 0x0004: Version
    smdh[0x0004:0x0006] = struct.pack('<H', 0)
    
    # 0x0006: Reserved
    smdh[0x0006:0x0008] = struct.pack('<H', 0)
    
    # 0x0008: 16 Application Title structs (0x200 bytes each = 0x2000 total)
    # Each struct: Short (0x80) + Long (0x100) + Publisher (0x80)
    # Languages: JP, EN, FR, DE, IT, ES, ZH_CN, KO, NL, PT, RU, ZH_TW, +4 reserved
    title_utf16 = title.encode('utf-16-le')
    desc_utf16 = description.encode('utf-16-le')
    author_utf16 = author.encode('utf-16-le')
    
    for lang_idx in range(16):
        base_offset = 0x0008 + (lang_idx * 0x200)
        
        # Short description (0x80 bytes)
        short_desc = title_utf16[:0x80].ljust(0x80, b'\x00')
        smdh[base_offset:base_offset+0x80] = short_desc
        
        # Long description (0x100 bytes)
        long_desc = desc_utf16[:0x100].ljust(0x100, b'\x00')
        smdh[base_offset+0x80:base_offset+0x180] = long_desc
        
        # Publisher (0x80 bytes)
        publisher = author_utf16[:0x80].ljust(0x80, b'\x00')
        smdh[base_offset+0x180:base_offset+0x200] = publisher
    
    # 0x2008: Application Settings (0x30 bytes)
    # Game Ratings (0x10 bytes)
    smdh[0x2008:0x2018] = b'\x00' * 0x10
    
    # Region Lockout (0x04 bytes) - 0x7FFFFFFF = region free
    smdh[0x2018:0x201C] = struct.pack('<I', 0x7FFFFFFF)
    
    # Match Maker IDs (0x0C bytes)
    smdh[0x201C:0x2028] = b'\x00' * 0x0C
    
    # Flags (0x04 bytes)
    # Bit 0: Visibility on Home Menu (1 = visible)
    # Bit 7: Uses save data (0 = no save)
    flags = 0x0001  # Just make it visible
    smdh[0x2028:0x202C] = struct.pack('<I', flags)
    
    # EULA Version (0x02 bytes)
    smdh[0x202C:0x202E] = b'\x00\x00'
    
    # Reserved (0x02 bytes)
    smdh[0x202E:0x2030] = b'\x00\x00'
    
    # Optimal Animation Default Frame (0x04 bytes)
    smdh[0x2030:0x2034] = struct.pack('<f', 0.0)
    
    # CEC (StreetPass) ID (0x04 bytes)
    smdh[0x2034:0x2038] = b'\x00\x00\x00\x00'
    
    # 0x2038: Reserved (0x08 bytes)
    smdh[0x2038:0x2040] = b'\x00' * 0x08
    
    # 0x2040: Small icon (24x24, RGB565, tiled)
    small_icon = icon.resize((24, 24), Image.Resampling.LANCZOS)
    small_icon_data = create_tiled_icon(small_icon, 24, 24)
    smdh[0x2040:0x24C0] = small_icon_data
    
    # 0x24C0: Large icon (48x48, RGB565, tiled)
    large_icon = icon.resize((48, 48), Image.Resampling.LANCZOS)
    large_icon_data = create_tiled_icon(large_icon, 48, 48)
    smdh[0x24C0:0x36C0] = large_icon_data
    
    # Write to file
    with open(output_path, 'wb') as f:
        f.write(smdh)
    
    print(f"SMDH created: {output_path} ({len(smdh)} bytes)")

if __name__ == '__main__':
    create_smdh(
        title="X18 Mixer",
        description="X18 Mixer Controller",
        author="MLJ",
        icon_path="/Users/lorenzomazzocchetti/Desktop/Github/X18_Nintendo_ds/gfx/icon.png",
        output_path="/Users/lorenzomazzocchetti/Desktop/Github/X18_Nintendo_ds/gfx/icon.smdh"
    )
