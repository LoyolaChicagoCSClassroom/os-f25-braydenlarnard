
#include <stdint.h>
#include "rprintf.h"
#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6


unsigned char keyboard_map[128] =
{
   0,  27, '1', '2', '3', '4', '5', '6', '7', '8',     /* 9 */
 '9', '0', '-', '=', '\b',     /* Backspace */
 '\t',                 /* Tab */
 'q', 'w', 'e', 'r',   /* 19 */
 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
   0,                  /* 29   - Control */
 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',     /* 39 */
'\'', '`',   0,                /* Left shift */
'\\', 'z', 'x', 'c', 'v', 'b', 'n',                    /* 49 */
 'm', ',', '.', '/',   0,                              /* Right shift */
 '*',
   0,  /* Alt */
 ' ',  /* Space bar */
   0,  /* Caps lock */
   0,  /* 59 - F1 key ... > */
   0,   0,   0,   0,   0,   0,   0,   0,  
   0,  /* < ... F10 */
   0,  /* 69 - Num lock*/
   0,  /* Scroll Lock */
   0,  /* Home key */
   0,  /* Up Arrow */
   0,  /* Page Up */
 '-',
   0,  /* Left Arrow */
   0,  
   0,  /* Right Arrow */
 '+',
   0,  /* 79 - End key*/
   0,  /* Down Arrow */
   0,  /* Page Down */
   0,  /* Insert Key */
   0,  /* Delete Key */
   0,   0,   0,  
   0,  /* F11 Key */
   0,  /* F12 Key */
   0,  /* All other keys are undefined */
};

const unsigned int multiboot_header[]  __attribute__((section(".multiboot"))) = {MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12};

uint8_t inb (uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

struct termbuf {
    char ascii;
    char color;
};

// Variables to track the cursor position
int px = 0;
int py = 0;

// Function that helps scroll the screen when text reaches end
void scroll() {
    struct termbuf *vram = (struct termbuf*) 0xb8000; // video mem
    int width = 80; // 80 columns
    int height = 25; // 25 rows

    // Shift rows up one
    for (int y = 0; y < height -1; y++) {
        for (int x = 0; x < width; x++) {
            vram[y*width+x] = vram [(y+1) * width + x];
        }
    }

    // Clear out the final row
    for (int x = 0; x < width; x++) {
        vram[(height-1) * width + x].ascii = ' ';
        vram[(height-1) * width + x].color = 7;
    }
}

// Function that helps print out characters
int putc(int data) {
    struct termbuf *vram = (struct termbuf*) 0xb8000; // Base address of video mem
    int width = 80; // Columns
    int height = 25; // Rows

    // Handle special character case
    if (data == '\n') {
        px = 0;
	py++;
    } else {
        // Write the character to the correct row & column
        vram[py * width + px].ascii = (char) data;
        vram[py * width + px].color = 7;
        px++; // Incremept row
    }

    // If it reaches the end, wrap it to the next line
    if (px >= width) {
        px = 0;
	py++;
    }

    // If it reaches the last row, scroll up
    if (py >= height) {
        scroll();
	py= height - 1;
    }

    return data;
}

void main() {
    //unsigned short *vram = (unsigned short*)0xb8000; // Base address of video mem
    //const unsigned char color = 7; // gray text on black background

    esp_printf(putc, "Hello world! \n"); // Prints to the screen

    while(1) {
        // Read the status register
        uint8_t status = inb(0x64);
 
        // Check the LSB to see if output buffer contains scancode
        if(status & 1) {
            uint8_t scancode = inb(0x60);
            (void)scancode;

	    // Print scancode to the terminal
	    esp_printf(putc, "Scancode: 0x%x\n", scancode);

	}
    }
}
