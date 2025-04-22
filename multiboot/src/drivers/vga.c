#include <vga.h>
#include <dumbstring.h>
#include <stdint-gcc.h>

static uint16_t curCursorIdx = 0;
static uint16_t* VGA_ptr = (uint16_t*)VGA_BASE_ADDR;

static inline void VGA_draw_cursor(VGAColorCode bkg) {
    uint16_t attribute = (bkg << 4) | (BLACK & 0x0F);
    VGA_ptr[curCursorIdx] = (attribute << 8) | '\0';
}

void VGA_clear() {
    curCursorIdx = 0;
    for (int i = 0; i < SCREEN_MAX_FLAT; i++) {
        put_char(BLACK, WHITE, ' ');
    }
    curCursorIdx = 0;

    VGA_draw_cursor(WHITE);
}
void VGA_display_char(char ch) {
    put_char(BLACK, WHITE, ch);
}
void VGA_display_str(const char * str) {
    uint32_t idx = 0;
    char ch = str[idx];
    while (ch != '\0') {
        put_char(BLACK, WHITE, ch);
        idx++;
        ch = str[idx];
    }
}

void put_char(VGAColorCode background, VGAColorCode foreground, char ch) {
    uint8_t backSpace = ch == '\b';
    uint8_t tabSpace = ch == '\t';
    uint8_t newLine = ch == '\n';
    uint8_t specialChar = backSpace || tabSpace || newLine;

    if (backSpace) {
        if (curCursorIdx % SCREEN_WIDTH == 0) return;
        
        VGA_draw_cursor(BLACK);
        curCursorIdx--;
        goto draw_cursor;
    }
    else if (tabSpace) {
        VGA_draw_cursor(BLACK);

        uint32_t spacesToMove = TAB_SPACES - curCursorIdx % TAB_SPACES;
        curCursorIdx += spacesToMove;
    }
    else if (newLine) {
        VGA_draw_cursor(BLACK);

        uint32_t charsLeft = SCREEN_WIDTH - curCursorIdx % SCREEN_WIDTH;
        curCursorIdx += charsLeft;
    }
    if (curCursorIdx >= SCREEN_MAX_FLAT) {
        memcpy(VGA_ptr, VGA_ptr + SCREEN_WIDTH, (SCREEN_MAX_FLAT-SCREEN_WIDTH) * sizeof(uint16_t));
        memset(VGA_ptr + (SCREEN_MAX_FLAT-SCREEN_WIDTH), '\0', SCREEN_WIDTH * sizeof(uint16_t));
        curCursorIdx = SCREEN_MAX_FLAT-SCREEN_WIDTH;
    }

    if (!specialChar) {
        uint16_t attribute = (background << 4) | (foreground & 0x0F);
        VGA_ptr[curCursorIdx] = (attribute << 8) | ch;
        curCursorIdx++;
    }

draw_cursor:
    VGA_draw_cursor(WHITE);
}
