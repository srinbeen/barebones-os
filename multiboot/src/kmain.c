#include <stdint-gcc.h>
#include <limits.h>

#include <vga.h>
#include <dumbio.h>
#include <keyboard.h>

#include <idt.h>
#include <pic.h>

void test_printk();
void test_keyboard_polling();

void kmain() {
  VGA_clear();

  __asm__ volatile ("cli");
  PS2_setup();
  PIC_remap(M_PIC_OFFSET, S_PIC_OFFSET);
  setup_idt();

  VGA_clear();
  printk("set up everything!!\n");
  __asm__ volatile ("sti");

  // __asm__ volatile ("int %0" : : "i" (100));
  // __asm__ volatile ("int %0" : : "i" (EXC_TS));


  while(1);
}

void test_keyboard_polling() {
  PS2_setup();
  printk("\n");
  while(1) {
    PS2_process_keyboard();
  }
}

void test_printk() {
  printk("%c\n", 'a'); // should be "a"
  printk("%c\n", 'Q'); // should be "Q"
  printk("%c\n", 256 + '9'); // Should be "9"
  printk("%s\n", "test string"); // "test string"
  printk("foo%sbar\n", "blah"); // "fooblahbar"
  printk("foo%%sbar\n"); // "foo%sbar"
  printk("%d\n", INT_MIN); // "-2147483648"
  printk("%d\n", INT_MAX); // "2147483647"
  printk("%u\n", 0); // "0"
  printk("%u\n", UINT_MAX); // "4294967295"
  printk("%x\n", 0xDEADbeef); // "deadbeef"
  printk("%p\n", (void*)UINTPTR_MAX); // "0xFFFFFFFFFFFFFFFF"
  printk("%hd\n", 0x8000); // "-32768"
  printk("%hd\n", 0x7FFF); // "32767"
  printk("%hu\n", 0xFFFF); // "65535"
  printk("%ld\n", LONG_MIN); // "-9223372036854775808"
  printk("%ld\n", LONG_MAX); // "9223372036854775807"
  printk("%lu\n", ULONG_MAX); // "18446744073709551615"
  printk("%qd\n", (long long)LONG_MIN); // "-9223372036854775808"
  printk("%qd\n", (long long)LONG_MAX); // "9223372036854775807"
  printk("%qu\n", (long long)ULONG_MAX); // "18446744073709551615"
}