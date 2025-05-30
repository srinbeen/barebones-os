#include <stdint-gcc.h>
#include <limits.h>

#include <vga.h>
#include <dumbio.h>
#include <keyboard.h>

#include <pic.h>
#include <interrupts.h>
#include <serial.h>

#include <multiboot.h>
#include <pageframe.h>
#include <pagetable.h>

void test_printk();
void test_keyboard_polling();
void pf_alloc_test(multiboot_fixed_header_t* mb_header);
void vp_alloc_test();

void kmain(multiboot_fixed_header_t* mb_header) {
  CLI();
    VGA_clear();
    pf_alloc_init(mb_header);
    setup_kernel_page_table();

    TSS_setup();
    PIC_remap(M_PIC_OFFSET, S_PIC_OFFSET);
    setup_idt();
    SER_init();
    // PS2_setup();
  STI();
  
  VGA_clear();

  // uint64_t* vp = MMU_alloc_page();
  // printk("vaddr: %p\n", vp);
  // for(int i = 0; i < PAGE_SIZE/sizeof(uint64_t); i++) {
  //   vp[i] = 0xDEADBEEF;
  // }
  // for(int i = 0; i < PAGE_SIZE/sizeof(uint64_t); i++) {
  //   printk("%d: %lx\n", i, vp[i]);
  // }

  vp_alloc_test();

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

void pf_alloc_test(multiboot_fixed_header_t* mb_header) {
  pf_alloc_init(mb_header);

  void* pf;
  for(int i = 1;;i++) {
    for (int j = 0; j < (1 << i); j++) {
      pf = pf_alloc();
      if (pf == NULL_PAGE) {
        break;
      }
    }
    if (pf != NULL_PAGE) {
        pf_free(pf);
    }
    else {
      break;
    }
  }

  printk("hell yeah!\n");
  pf_alloc_init(mb_header);
  
  uintptr_t* prev;
  for(int i = 0;;i++) {
    uintptr_t* bruh = pf_alloc();
    if (bruh == NULL_PAGE) {
      break;
    }

    for (int j = 0; j < (PAGE_SIZE/sizeof(uintptr_t)); j++) {
      bruh[j] = (uintptr_t)bruh;
    }

    prev = bruh;
  }

  for (uintptr_t i = 0; i <= (uintptr_t)prev; i += PAGE_SIZE) {
    if (!is_pf_valid((void*)i)) {
      continue;
    }

    for (int j = 0; j < (PAGE_SIZE/sizeof(uintptr_t)); j++) {
      if (((uintptr_t*)i)[j] != i) {
        printk("BRUH BRUH BRUH\n");
        HLT();
      }
    }     
  }

  void* alloced_mem[4];
  pf_alloc_init(mb_header);

  for (int i = 0; i < 4; i++) {
    alloced_mem[i] = pf_alloc();
    printk("alloced addr: %p\n", alloced_mem[i]);
  }
  for (int i = 0; i < 4; i++) {
    pf_free(alloced_mem[i]);
  }
  for (int i = 0; i < 4; i++) {
    void* temp = pf_alloc();
    printk("newly alloced addr: %p\n", temp);
  }
  
  printk("TEST PASSED!!!\n");
}

void vp_alloc_test() {

  void* vp;
  for(int i = 1;;i++) {
    for (int j = 0; j < (1 << i); j++) {
      vp = MMU_alloc_page();
      if (j % 2 == 0) *(uintptr_t*)vp = (uintptr_t)vp;
    }
    MMU_free_page(vp);
  }

  printk("TEST PASSED!!!\n");
}