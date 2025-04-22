#ifndef PS2_H
#define PS2_H

#include <stdint-gcc.h>

#define PS2_DATA_PORT           0x60
#define PS2_STATUS_REG          0x64
#define PS2_CMD_REG             0x64

#define PS2_CMD_CCB_READ        0x20
#define PS2_CMD_CCB_WRITE       0x60
#define PS2_CMD_RESET           0xFF

#define PS2_CMD_SELF_TEST       0xAA
#define PS2_CMD_SELF_TEST_OK    0x55
#define PS2_CMD_SELF_TEST_FAIL  0xFC


#define PS2_CMD_ENABLE_1        0xAE
#define PS2_CMD_DISABLE_1       0xAD
#define PS2_CMD_TEST_1          0xAB
#define PS2_CMD_TEST_1_OK       0x00
                                
#define PS2_CMD_ENABLE_2        0xA8
#define PS2_CMD_DISABLE_2       0xA7

#define SCAN_CODE_LSHIFT        0x12
#define SCAN_CODE_RSHIFT        0x59
#define SCAN_CODE_RELEASE       0xF0

void PS2_setup();
void PS2_write_cmd(uint8_t cmd);
void PS2_write_data(uint8_t data);
uint8_t PS2_read_data();
void PS2_process_keyboard();

extern const char scancode_map_unshifted[];
extern const char scancode_map_shifted[];

#endif // PS2_H