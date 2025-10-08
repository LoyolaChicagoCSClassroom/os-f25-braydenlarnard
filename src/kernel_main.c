#include <stdint.h>
#include "rprintf.h"
#include "interrupt.h"
#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6


const unsigned int multiboot_header[]  __attribute__((section(".multiboot"))) = {MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12};

uint8_t inb (uint16_t _port);

// A termbuf struct to wrap each screen character in
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

    // a bunch of support functions to enable interrupts
    // Initialize interrupt system for keyboard input
    remap_pic();  // Set up interrupt controller
    load_gdt();   // Load the global descriptor table
    init_idt();   // Initialize the interrupt descriptor table
    asm("sti");   // Enable interrupts

    // Infinite loop - wait for keyboard interrupts
    while(1) {
        asm("hlt");  // Halt until next interrupt
    }
}   

