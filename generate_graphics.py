#!/usr/bin/env python3
"""
Generate modern 3D style graphics for X18 Mixer UI
Dark palette with high contrast for low-light usage
"""

from PIL import Image, ImageDraw, ImageFilter
import os

# Color palette - Dark with high contrast
COLORS = {
    'bg_dark': (13, 13, 18),           # Very dark blue-black
    'bg_medium': (32, 32, 42),         # Medium dark
    'bg_light': (50, 50, 65),          # Light dark
    'border_bright': (200, 200, 255),  # Bright blue
    'border_cyan': (0, 255, 255),      # Cyan
    'border_yellow': (255, 255, 0),    # Yellow
    'border_orange': (255, 153, 0),    # Orange
    'border_green': (0, 255, 0),       # Green
    'text_primary': (255, 255, 255),   # White
    'text_secondary': (200, 200, 200), # Light gray
    'shadow_dark': (0, 0, 0),          # Black for shadows
    'highlight': (100, 200, 255),      # Light blue
}

def draw_3d_border(draw, x, y, w, h, color_bright, color_dark, thickness=2):
    """Draw 3D beveled border effect"""
    # Top-left bright
    for i in range(thickness):
        draw.line([(x+i, y+i), (x+w-1-i, y+i)], fill=color_bright, width=1)
        draw.line([(x+i, y+i), (x+i, y+h-1-i)], fill=color_bright, width=1)
    # Bottom-right dark
    for i in range(thickness):
        draw.line([(x+i, y+h-1-i), (x+w-1-i, y+h-1-i)], fill=color_dark, width=1)
        draw.line([(x+w-1-i, y+i), (x+w-1-i, y+h-1-i)], fill=color_dark, width=1)

def draw_button_3d(draw, x, y, w, h, label, color_bg, color_border):
    """Draw 3D button with label"""
    # Main button area
    draw.rectangle([x, y, x+w-1, y+h-1], fill=color_bg)
    # 3D effect borders
    draw_3d_border(draw, x, y, w, h, color_border, COLORS['shadow_dark'], 2)
    # Inner highlight
    draw.line([(x+2, y+2), (x+w-3, y+2)], fill=COLORS['highlight'], width=1)

def create_background_panel(filename, width, height, color_main=None):
    """Create a modern 3D panel background"""
    if color_main is None:
        color_main = COLORS['bg_light']
    
    img = Image.new('RGBA', (width, height), COLORS['bg_dark'])
    draw = ImageDraw.Draw(img)
    
    # Main panel
    draw.rectangle([0, 0, width-1, height-1], fill=color_main)
    
    # Outer border (bright)
    draw_3d_border(draw, 0, 0, width, height, COLORS['border_bright'], COLORS['shadow_dark'], 2)
    
    # Inner secondary border
    draw_3d_border(draw, 2, 2, width-4, height-4, COLORS['bg_light'], COLORS['bg_medium'], 1)
    
    img.save(filename)
    print(f"✓ Created {filename}")

def create_panel_header(filename, width, height, title_hint=""):
    """Create a header panel for window titles"""
    img = Image.new('RGBA', (width, height), COLORS['bg_dark'])
    draw = ImageDraw.Draw(img)
    
    # Gradient-like effect with rectangles
    for i in range(height):
        ratio = i / height
        r = int(COLORS['bg_medium'][0] + (COLORS['bg_light'][0] - COLORS['bg_medium'][0]) * ratio)
        g = int(COLORS['bg_medium'][1] + (COLORS['bg_light'][1] - COLORS['bg_medium'][1]) * ratio)
        b = int(COLORS['bg_medium'][2] + (COLORS['bg_light'][2] - COLORS['bg_medium'][2]) * ratio)
        draw.line([(0, i), (width, i)], fill=(r, g, b))
    
    # Bottom bright line for depth
    draw.line([(0, height-2), (width, height-2)], fill=COLORS['border_cyan'], width=2)
    
    img.save(filename)
    print(f"✓ Created {filename}")

def create_button_set(filename, width, height, num_buttons=4, button_color=None):
    """Create a set of 3D buttons"""
    if button_color is None:
        button_color = COLORS['bg_medium']
    
    img = Image.new('RGBA', (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Create button areas (simple rectangular buttons)
    button_w = (width - 5) // num_buttons
    button_h = height - 4
    
    for i in range(num_buttons):
        btn_x = i * (button_w + 1) + 2
        btn_y = 2
        draw.rectangle([btn_x, btn_y, btn_x+button_w-1, btn_y+button_h-1], fill=button_color)
        draw_3d_border(draw, btn_x, btn_y, button_w, button_h, COLORS['border_bright'], COLORS['shadow_dark'], 2)
    
    img.save(filename)
    print(f"✓ Created {filename}")

def create_input_field(filename, width, height):
    """Create input field texture"""
    img = Image.new('RGBA', (width, height), COLORS['bg_dark'])
    draw = ImageDraw.Draw(img)
    
    # Field background
    draw.rectangle([0, 0, width-1, height-1], fill=COLORS['bg_dark'])
    
    # Recessed effect (inset border)
    draw_3d_border(draw, 0, 0, width, height, COLORS['shadow_dark'], COLORS['border_cyan'], 2)
    
    # Subtle inner line
    draw.rectangle([2, 2, width-3, height-3], outline=COLORS['border_bright'], width=1)
    
    img.save(filename)
    print(f"✓ Created {filename}")

def create_list_item_bg(filename, width, height, selected=False):
    """Create list item background"""
    color = COLORS['border_cyan'] if selected else COLORS['bg_medium']
    img = Image.new('RGBA', (width, height), COLORS['bg_dark'])
    draw = ImageDraw.Draw(img)
    
    draw.rectangle([0, 0, width-1, height-1], fill=color if selected else COLORS['bg_medium'])
    
    if selected:
        # Bright border for selected
        draw_3d_border(draw, 0, 0, width, height, COLORS['border_yellow'], COLORS['shadow_dark'], 1)
    else:
        # Subtle border
        draw.rectangle([0, 0, width-1, height-1], outline=COLORS['bg_light'], width=1)
    
    # Left accent bar
    draw.rectangle([0, 0, 3, height-1], fill=color if selected else COLORS['bg_light'])
    
    img.save(filename)
    print(f"✓ Created {filename}")

def create_keyboard_key(filename, width, height, key_type='normal'):
    """Create keyboard key texture"""
    img = Image.new('RGBA', (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    if key_type == 'normal':
        color_bg = COLORS['bg_medium']
        color_border = COLORS['border_bright']
    elif key_type == 'special':
        color_bg = COLORS['border_orange']
        color_border = (255, 200, 0)
    else:  # action
        color_bg = COLORS['border_cyan']
        color_border = (0, 200, 255)
    
    # Key button
    draw.rectangle([1, 1, width-2, height-2], fill=color_bg)
    draw_3d_border(draw, 1, 1, width-2, height-2, color_border, COLORS['shadow_dark'], 1)
    
    img.save(filename)
    print(f"✓ Created {filename}")

def create_indicator_bar(filename, width, height, bar_type='level'):
    """Create indicator/progress bar texture"""
    img = Image.new('RGBA', (width, height), COLORS['bg_dark'])
    draw = ImageDraw.Draw(img)
    
    # Container
    draw.rectangle([0, 0, width-1, height-1], fill=COLORS['bg_medium'])
    draw_3d_border(draw, 0, 0, width, height, COLORS['border_bright'], COLORS['shadow_dark'], 2)
    
    # Fill area (partial)
    fill_w = int((width - 4) * 0.7)  # 70% filled for demo
    if bar_type == 'level':
        draw.rectangle([2, 2, 2+fill_w, height-3], fill=COLORS['border_green'])
    else:  # energy
        draw.rectangle([2, 2, 2+fill_w, height-3], fill=COLORS['border_yellow'])
    
    img.save(filename)
    print(f"✓ Created {filename}")

def main():
    gfx_dir = './gfx'
    os.makedirs(gfx_dir, exist_ok=True)
    
    print("🎨 Generating modern 3D UI graphics...\n")
    
    # ===== NETWORK CONFIG WINDOW =====
    print("📡 Network Config Window:")
    create_panel_header(f'{gfx_dir}/header_network.png', 320, 40, "Network Settings")
    create_background_panel(f'{gfx_dir}/panel_network_bg.png', 320, 180, COLORS['bg_light'])
    create_input_field(f'{gfx_dir}/input_field.png', 200, 30)
    create_button_set(f'{gfx_dir}/button_row_3.png', 320, 20, 3)
    
    # ===== SHOW MANAGER WINDOW =====
    print("\n📋 Show Manager Window:")
    create_panel_header(f'{gfx_dir}/header_manager.png', 320, 40, "Show Manager")
    create_background_panel(f'{gfx_dir}/panel_manager_bg.png', 320, 180, COLORS['bg_light'])
    create_list_item_bg(f'{gfx_dir}/list_item_normal.png', 300, 18, False)
    create_list_item_bg(f'{gfx_dir}/list_item_selected.png', 300, 18, True)
    create_button_set(f'{gfx_dir}/button_row_4.png', 320, 20, 4)
    
    # ===== OPTIONS WINDOW =====
    print("\n⚙️  Options Window:")
    create_panel_header(f'{gfx_dir}/header_options.png', 320, 40, "Options")
    create_background_panel(f'{gfx_dir}/panel_options_bg.png', 320, 180, COLORS['bg_light'])
    create_indicator_bar(f'{gfx_dir}/slider_bg.png', 250, 15, 'level')
    
    # ===== INFO PANEL =====
    print("\n📊 Info Panel:")
    create_panel_header(f'{gfx_dir}/header_info.png', 320, 30, "Information")
    create_background_panel(f'{gfx_dir}/panel_info_bg.png', 320, 90, COLORS['bg_medium'])
    
    # ===== KEYBOARD =====
    print("\n⌨️  Keyboard:")
    create_keyboard_key(f'{gfx_dir}/key_normal.png', 48, 40, 'normal')
    create_keyboard_key(f'{gfx_dir}/key_special.png', 48, 40, 'special')
    create_keyboard_key(f'{gfx_dir}/key_action.png', 48, 40, 'action')
    create_keyboard_key(f'{gfx_dir}/key_wide.png', 95, 40, 'special')
    
    print("\n✅ All graphics generated successfully!\n")

if __name__ == '__main__':
    main()
