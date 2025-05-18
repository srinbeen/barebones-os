#include <serial.h>
#include <dumbio.h>

#include <pic.h>
#include <interrupts.h>

static void SER_consume(serial_state_t* state);

static serial_state_t state;

void SER_init() {
   outb(COM1_IER, 0x00);                    // disable interrupts
   outb(COM1_LCR, 0x80);                    // Enable DLAB (set baud rate divisor)
   outb(COM1, 0x03);                        // Set divisor to 3 (lo byte) 38400 baud
   outb(COM1 + 1, 0x00);                    //                  (hi byte)
   outb(COM1_LCR, 0x03);                    // 8 bits, no parity, one stop bit
   outb(COM1_IER, (1 << 1) | (1 << 2));     // enable TX and LINE intr


   state.head = 0;
   state.tail = 0;

   register_irq(IRQ_SERIAL, (void*)&state, SER_handler);
   PIC_enable_line(PIC_SERIAL_IRQ_NUM);
}

void SER_handler(int n, int e, void* args) {
    serial_state_t* state = (serial_state_t*)args;
    SERIAL_INT_STATE int_state = (SERIAL_INT_STATE)INT_STATE_BITS(inb(COM1_IIR));

    switch (int_state) {
        case LINE_STATUS: {
            inb(COM1_LSR);
            break;
        }
        case TX_EMPTY: {
            if (state->head == state->tail) {
                // empty buffer, nothing to write
            }
            else {
                SER_consume(state);
            }
            break;
        }
        default: {
            break;
        }

        PIC_sendEOI(PIC_SERIAL_IRQ_NUM);
    }
}

void SER_write(char c) {
    CLI();
    if (BUF_NEXT(state.tail) == state.head) {
        // full buffer, cannot write
    }
    else {
        state.buffer[state.tail] = c;
        state.tail = BUF_NEXT(state.tail);
    }

    if (LSR_THRE(inb(COM1_LSR))) {
        SER_consume(&state);
    }
    STI();
}

static void SER_consume(serial_state_t* state) {
    char out = state->buffer[state->head];
    outb(COM1,out);
    state->head = BUF_NEXT(state->head);
}