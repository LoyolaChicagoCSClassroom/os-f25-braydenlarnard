
#include <stdint.h>
#include "rprintf.h"
#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

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
int px = 0;
int py = 0;
int putc(int data) {
    struct termbuf *vram = (struct termbuf*) 0xb8000; // Base address of video mem
    int width = 80; // Columns
    int height = 25; // Rows

    // Write the character to the correct row & column
    vram[py * width + px].ascii = data;
    vram[py * width + px].color = 7;
    px++; // Incremept row

    // If it reaches the end, wrap it to the next line
    if (py >= width) {
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

    esp_printf(putc, "Hello world!"); // Prints to the screen

    while(1) {
        uint8_t status = inb(0x64);

        if(status & 1) {
            uint8_t scancode = inb(0x60);
            (void)scancode;
	}
    }
}
