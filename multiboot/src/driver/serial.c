#include <serial.h>
#include <dumbio.h>

#include <pic.h>
#include <interrupts.h>

void SER_init() {
   outb(COM1_IER, 0x00);    // disable interrupts
   outb(COM1_LCR, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(COM1, 0x03);        // Set divisor to 3 (lo byte) 38400 baud
   outb(COM1 + 1, 0x00);    //                  (hi byte)
   outb(COM1_LCR, 0x03);    // 8 bits, no parity, one stop bit
   outb(COM1_IER, 0x01);    // enable interrupts

   register_irq(IRQ_SERIAL, (void*)0, SER_handler);
   PIC_enable_line(PIC_SERIAL_IRQ_NUM);
}

void SER_handler(void* args) {

}