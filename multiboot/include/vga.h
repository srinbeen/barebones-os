#ifndef VGA_H
#define VGA_H

#define VGA_BASE_ADDR 0xb8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define SCREEN_MAX_FLAT (SCREEN_WIDTH*SCREEN_HEIGHT)
#define TAB_SPACES 4

#define ASCII_SPACE 0x20

typedef enum e_VGAColorCode {
    BLACK = 0,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    LIGHT_GRAY,
    DARK_GRAY,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    LIGHT_MAGENTA,
    YELLOW,
    WHITE
} VGAColorCode;

// The VGA console is memory-mapped at address 0xb8000.
// Each character is stored as one 16 bit value in row-major order.
// The entire screen is 80x25. 

void VGA_clear(void);
void VGA_display_char(char ch);
void VGA_display_str(const char * str);

void put_char(VGAColorCode background, VGAColorCode foreground, char ch);

#endif // VGA_H