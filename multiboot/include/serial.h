#ifndef SERIAL_H
#define SERIAL_H

#include <stdbool.h>
#include <stdint-gcc.h>

#define COM1        0x3f8
#define COM1_IER    (COM1 + 1)
#define COM1_IIR    (COM1 + 2)
#define COM1_LCR    (COM1 + 3)
#define COM1_LSR    (COM1 + 5)

#define NUM_ELEM            8
#define BUF_SIZE            (NUM_ELEM + 1)
#define BUF_NEXT(x)         ((x + 1) % BUF_SIZE)

#define INT_STATE_BITS(x)   ((0b00000110 & x) >> 1)

#define LSR_THRE_BIT_NUM        5
#define LSR_THRE(x)             (((1 << LSR_THRE_BIT_NUM) & x) >> LSR_THRE_BIT_NUM)


typedef struct {
    uint8_t buffer[BUF_SIZE];
    int head;
    int tail;
} serial_state_t;

typedef enum {
    MODEM_STATUS,
    TX_EMPTY,
    RX_AVL,
    LINE_STATUS
} SERIAL_INT_STATE;

void SER_init();
void SER_handler(int n, int e, void* args);
void SER_write(char c);

#endif // SERIAL_H