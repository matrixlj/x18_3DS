#!/usr/bin/env python3
"""
Create texture atlases and update rendering code
Converts individual PNG assets into efficient textures for 3DS
"""

from PIL import Image
import os

def create_texture_atlas(output_name, images_list, cols=4):
    """Create a texture atlas from multiple images"""
    if not images_list:
        return
    
    # Calculate grid
    rows = (len(images_list) + cols - 1) // cols
    
    # Load first image to get dimensions (assume uniform)
    first_img = Image.open(images_list[0])
    img_w, img_h = first_img.size
    
    # Create atlas
    atlas_w = cols * img_w
    atlas_h = rows * img_h
    atlas = Image.new('RGBA', (atlas_w, atlas_h), (0, 0, 0, 0))
    
    # Place images
    for idx, img_path in enumerate(images_list):
        img = Image.open(img_path)
        row = idx // cols
        col = idx % cols
        x = col * img_w
        y = row * img_h
        atlas.paste(img, (x, y))
    
    atlas.save(output_name)
    print(f"✓ Created atlas: {output_name} ({atlas_w}x{atlas_h})")
    return atlas_w, atlas_h

def main():
    gfx_dir = './gfx'
    os.makedirs(gfx_dir, exist_ok=True)
    
    print("🎨 Creating texture atlases...\n")
    
    # Headers atlas
    headers = [
        f'{gfx_dir}/header_network.png',
        f'{gfx_dir}/header_manager.png',
        f'{gfx_dir}/header_options.png',
        f'{gfx_dir}/header_info.png',
    ]
    create_texture_atlas(f'{gfx_dir}/headers_atlas.png', headers, cols=1)
    
    # Panels atlas
    panels = [
        f'{gfx_dir}/panel_network_bg.png',
        f'{gfx_dir}/panel_manager_bg.png',
        f'{gfx_dir}/panel_options_bg.png',
        f'{gfx_dir}/panel_info_bg.png',
    ]
    create_texture_atlas(f'{gfx_dir}/panels_atlas.png', panels, cols=2)
    
    # UI elements atlas
    ui_elements = [
        f'{gfx_dir}/input_field.png',
        f'{gfx_dir}/button_row_3.png',
        f'{gfx_dir}/button_row_4.png',
        f'{gfx_dir}/list_item_normal.png',
        f'{gfx_dir}/list_item_selected.png',
        f'{gfx_dir}/slider_bg.png',
    ]
    create_texture_atlas(f'{gfx_dir}/ui_elements_atlas.png', ui_elements, cols=2)
    
    # Keyboard atlas
    keyboard = [
        f'{gfx_dir}/key_normal.png',
        f'{gfx_dir}/key_special.png',
        f'{gfx_dir}/key_action.png',
        f'{gfx_dir}/key_wide.png',
    ]
    create_texture_atlas(f'{gfx_dir}/keyboard_atlas.png', keyboard, cols=2)
    
    print("\n✅ All texture atlases created successfully!\n")

if __name__ == '__main__':
    main()
