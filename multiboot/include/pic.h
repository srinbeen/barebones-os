#ifndef PIC_H
#define PIC_H

#include <dumbio.h>

#define M_PIC_CMD               0x20
#define M_PIC_DATA              0x21
#define S_PIC_CMD               0xA0
#define S_PIC_DATA              0xA1

#define ICW1_ICW4	            0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	            0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	        0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	            0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	            0x10		/* Initialization - required! */

#define ICW4_8086	            0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	            0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	        0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	        0x0C		/* Buffered mode/master */
#define ICW4_SFNM	            0x10		/* Special fully nested (not) */

#define PIC_CMD_EOI             0x20

#define M_PIC_OFFSET            0x20 
#define S_PIC_OFFSET            (M_PIC_OFFSET + 8) 

#define PIC_KEYBOARD_IRQ_NUM    1

void PIC_remap(int offset1, int offset2);
void io_wait();
void PIC_sendEOI(uint8_t irq);
void PIC_disable_line(uint8_t irq);
void PIC_enable_line(uint8_t irq);
void PIC_disable();

#endif // PIC_H