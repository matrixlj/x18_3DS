// Bare-metal Nintendo 3DS - Versione semplificata
// Questo è un programma che gira senza dipendenze da libctru

// Indirizzi di memoria per 3DS
#define ARM9_ITCM_BASE      0x00000000
#define ARM9_DTCM_BASE      0x0B000000
#define IO_BASE             0x10000000
#define LCD_BASE            0x10202000
#define VRAM_BASE           0x18000000

// Registro di controllo del display
#define LCD_POWERCNT        ((volatile unsigned int*)(LCD_BASE + 0x10))
#define LCD_COLORFILL       ((volatile unsigned int*)(LCD_BASE + 0x14))

// Registro per il framebuffer (TOP screen)
#define LCD_TOP_FB1         ((volatile unsigned int*)(LCD_BASE + 0x68))
#define LCD_TOP_FB2         ((volatile unsigned int*)(LCD_BASE + 0x6C))

// Funzione per disegnare un pixel su 3DS (400x240, RGB565)
void draw_pixel(int x, int y, unsigned short color) {
    volatile unsigned short* vram = (volatile unsigned short*)VRAM_BASE;
    if (x >= 0 && x < 400 && y >= 0 && y < 240) {
        // 3DS ha layout lineare per il framebuffer
        vram[y * 400 + x] = color;
    }
}

// Colori RGB565 (formato nativo 3DS)
#define COLOR_WHITE   0xFFFF
#define COLOR_BLACK   0x0000
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0

// Disegna un rettangolo pieno
void fill_rect(int x1, int y1, int x2, int y2, unsigned short color) {
    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            draw_pixel(x, y, color);
        }
    }
}

// Routine di sincronizzazione con il refresh dello schermo
void wait_vblank(void) {
    static volatile unsigned int* gpu_stat = (volatile unsigned int*)0x10400010;
    while (*gpu_stat & 0x01);
    while (!(*gpu_stat & 0x01));
}

int main(void) {
    // Abilita i LED/display (assicurati che il display sia acceso)
    *LCD_POWERCNT |= 0x01000001;  // Abilita entrambi gli schermi
    
    // Pulisci il framebuffer con il colore di fill
    fill_rect(0, 0, 399, 239, COLOR_BLACK);
    
    // Disegna alcuni rettangoli per testare il display
    fill_rect(50, 20, 350, 80, COLOR_BLUE);
    fill_rect(50, 100, 350, 160, COLOR_GREEN);
    fill_rect(100, 180, 300, 220, COLOR_RED);
    
    // Disegna testo semplice (pattern per rappresentare "Hello 3DS")
    // Nota: per vero testo font rendering serve libctru
    
    // Loop infinito - il programma rimane attivo
    unsigned int frame = 0;
    while (1) {
        wait_vblank();
        frame++;
        
        // Potresti usare 'frame' per animazioni
        // Esempio: flash color ogni 30 frame
        if (frame % 60 == 0) {
            unsigned short flash_color = (frame / 60) % 2 ? COLOR_YELLOW : COLOR_WHITE;
            fill_rect(160, 210, 240, 235, flash_color);
        }
    }
    
    return 0;
}
