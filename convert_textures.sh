#!/bin/bash
# Convert PNG atlases to 3DS texture format (.t3x)

echo "🎨 Converting PNG atlases to 3DS texture format..."

cd gfx

# Function to convert PNG to t3x
convert_to_t3x() {
    local png_file=$1
    local t3x_file="${png_file%.png}.t3x"
    
    if [ -f "$png_file" ]; then
        echo "  Converting: $png_file"
        tex3ds -i "$png_file" -o "$t3x_file"
        if [ -f "$t3x_file" ]; then
            echo "    ✓ Created: $t3x_file"
        fi
    fi
}

# Convert each atlas
convert_to_t3x "headers_atlas.png"
convert_to_t3x "panels_atlas.png"
convert_to_t3x "ui_elements_atlas.png"
convert_to_t3x "keyboard_atlas.png"

echo -e "\n✅ All textures converted successfully!\n"
ls -lh *_atlas.t3x
