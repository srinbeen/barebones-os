#ifndef SERIAL_H
#define SERIAL_H

#define COM1        0x3f8
#define COM1_IER    (COM1 + 1)
#define COM1_LCR    (COM1 + 3)

void SER_init();
void SER_handler(void* args);

#endif // SERIAL_H