#include <pic.h>
#include <dumbio.h>

void io_wait() {
    outb(0x80, 0);
}
void PIC_remap(int offset1, int offset2) {
	outb(M_PIC_CMD, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	io_wait();
	outb(S_PIC_CMD, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(M_PIC_DATA, offset1);                 // ICW2: Master PIC vector offset
	io_wait();
	outb(S_PIC_DATA, offset2);                 // ICW2: Slave PIC vector offset
	io_wait();
	outb(M_PIC_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	io_wait();
	outb(S_PIC_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();
	
	outb(M_PIC_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	io_wait();
	outb(S_PIC_DATA, ICW4_8086);
	io_wait();

	PIC_disable();
}
void PIC_sendEOI(uint8_t irq) {
	if(irq >= 8) {
		outb(S_PIC_CMD, PIC_CMD_EOI);
    }
	
	outb(M_PIC_CMD, PIC_CMD_EOI);
}

void PIC_disable() {
	outb(M_PIC_DATA, 0xFF);
	outb(S_PIC_DATA, 0xFF);
}

void PIC_disable_line(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if(irq < 8) {
        port = M_PIC_DATA;
    } else {
        port = S_PIC_DATA;
        irq -= 8;
    }
    value = inb(port) | (1 << irq);
    outb(port, value);        
}

void PIC_enable_line(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if(irq < 8) {
        port = M_PIC_DATA;
    } else {
        port = S_PIC_DATA;
        irq -= 8;
    }
    value = inb(port) & ~(1 << irq);
    outb(port, value);        
}