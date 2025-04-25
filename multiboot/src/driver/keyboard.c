#include <keyboard.h>
#include <stdint-gcc.h>
#include <dumbio.h>

const char scancode_map_unshifted[256] = {
    [0x1C] = 'a',
    [0x32] = 'b',
    [0x21] = 'c',
    [0x23] = 'd',
    [0x24] = 'e',
    [0x2B] = 'f',
    [0x34] = 'g',
    [0x33] = 'h',
    [0x43] = 'i',
    [0x3B] = 'j',
    [0x42] = 'k',
    [0x4B] = 'l',
    [0x3A] = 'm',
    [0x31] = 'n',
    [0x44] = 'o',
    [0x4D] = 'p',
    [0x15] = 'q',
    [0x2D] = 'r',
    [0x1B] = 's',
    [0x2C] = 't',
    [0x3C] = 'u',
    [0x2A] = 'v',
    [0x1D] = 'w',
    [0x22] = 'x',
    [0x35] = 'y',
    [0x1A] = 'z',
    [0x45] = '0',
    [0x16] = '1',
    [0x1E] = '2',
    [0x26] = '3',
    [0x25] = '4',
    [0x2E] = '5',
    [0x36] = '6',
    [0x3D] = '7',
    [0x3E] = '8',
    [0x46] = '9',
    [0x0E] = '`',
    [0x4E] = '-',
    [0x55] = '=',
    [0x5D] = '\\',
    [0x66] = '\b',
    [0x0D] = '\t',
    [0x29] = ' ',
    [0x5A] = '\n',
    [0x54] = '[',
    [0x5B] = ']',
    [0x4C] = ';',
    [0x52] = '\'',
    [0x41] = ',',
    [0x49] = '.',
    [0x4A] = '/'
};
const char scancode_map_shifted[256] = {
    [0x1C] = 'A',
    [0x32] = 'B',
    [0x21] = 'C',
    [0x23] = 'D',
    [0x24] = 'E',
    [0x2B] = 'F',
    [0x34] = 'G',
    [0x33] = 'H',
    [0x43] = 'I',
    [0x3B] = 'J',
    [0x42] = 'K',
    [0x4B] = 'L',
    [0x3A] = 'M',
    [0x31] = 'N',
    [0x44] = 'O',
    [0x4D] = 'P',
    [0x15] = 'Q',
    [0x2D] = 'R',
    [0x1B] = 'S',
    [0x2C] = 'T',
    [0x3C] = 'U',
    [0x2A] = 'V',
    [0x1D] = 'W',
    [0x22] = 'X',
    [0x35] = 'Y',
    [0x1A] = 'Z',
    [0x45] = ')',
    [0x16] = '!',
    [0x1E] = '@',
    [0x26] = '#',
    [0x25] = '$',
    [0x2E] = '%',
    [0x36] = '^',
    [0x3D] = '&',
    [0x3E] = '*',
    [0x46] = '(',
    [0x0E] = '~',
    [0x4E] = '_',
    [0x55] = '+',
    [0x5D] = '|',
    [0x0D] = '\t',
    [0x66] = '\b',
    [0x29] = ' ',
    [0x5A] = '\n',
    [0x54] = '{',
    [0x5B] = '}',
    [0x4C] = ':',
    [0x52] = '"',
    [0x41] = '<',
    [0x49] = '>',
    [0x4A] = '?'
};

const char* cur_scancode_map;

inline uint8_t PS2_read_data() {
    while((inb(PS2_STATUS_REG) & 0x01) == 0x00);
    return inb(PS2_DATA_PORT);
}
inline void PS2_write_data(uint8_t data) {
    while((inb(PS2_STATUS_REG) & 0x02) == 0x02);
    outb(PS2_DATA_PORT, data);
    return;
}
inline void PS2_write_cmd(uint8_t cmd) {
    while((inb(PS2_STATUS_REG) & 0x02) == 0x02);
    outb(PS2_CMD_REG, cmd);
    return;
}

void PS2_setup() {
    PS2_write_cmd(PS2_CMD_DISABLE_1);
    PS2_write_cmd(PS2_CMD_DISABLE_2);
    printk("disabled ps2 ports\n");

    // flush output buffer
    inb(PS2_DATA_PORT);
    printk("flushed output buffer\n");

    // control configuration byte
    PS2_write_cmd(PS2_CMD_CCB_READ);
    uint8_t ccb = PS2_read_data();
    printk("ccb: %hx\n", ccb);
    ccb &= ~(uint8_t)(
        // port 1 disable interrupt
        (1 << 0) |
        // port 2 disable interrupt
        (1 << 1) |
        // port 1 enable clock
        (1 << 4) |
        // port 2 disable clock
        (1 << 5) |
        // port 1 disable translation
        (1 << 6)
    );
    PS2_write_cmd(PS2_CMD_CCB_WRITE);
    PS2_write_data(ccb);
    printk("wrote to ccb\n");

    // self test
    PS2_write_cmd(PS2_CMD_SELF_TEST);
    uint8_t testStatus = PS2_read_data();
    if (testStatus != PS2_CMD_SELF_TEST_OK) {
        printk("self test FAILED. testStatus: %hx\n", testStatus);
    }
    else {
        printk("self test PASSED\n");
    }
    // write back ccb (sometimes it's cleared when self-testing)
    PS2_write_cmd(PS2_CMD_CCB_WRITE);
    PS2_write_data(ccb);

    PS2_write_cmd(PS2_CMD_TEST_1);
    testStatus = PS2_read_data();
    if (testStatus != PS2_CMD_TEST_1_OK) {
        printk("port 1 test FAILED\n");
    }
    else {
        printk("port 1 test PASSED\n");
    }

    PS2_write_cmd(PS2_CMD_ENABLE_1);
    printk("port 1 enabled\n");

    PS2_write_cmd(PS2_CMD_CCB_READ);
    ccb = PS2_read_data();
    ccb |= (uint8_t)(
        // port 1 enable interrupt
        (1 << 0)
    );

    PS2_write_cmd(PS2_CMD_CCB_WRITE);
    PS2_write_data(ccb);


    PS2_write_cmd(PS2_CMD_RESET);

    cur_scancode_map = scancode_map_unshifted;
}

uint8_t PS2_isShift_scancode(uint8_t scancode) {
    if (scancode == SCAN_CODE_LSHIFT) {
        return 1;
    }
    else if (scancode == SCAN_CODE_RSHIFT) {
        return 2;
    }
    else {
        return 0;
    }
}

void PS2_process_keyboard() {
    uint8_t scancode = PS2_read_data();

    if (scancode == SCAN_CODE_RELEASE) {
        uint8_t scancode_released = PS2_read_data();
        
        if (PS2_isShift_scancode(scancode_released)) {
            cur_scancode_map = scancode_map_unshifted;
        }
    }
    else if (PS2_isShift_scancode(scancode)) {
        cur_scancode_map = scancode_map_shifted;
    }
    else {
        printk("%c", cur_scancode_map[scancode]);
    }
}
