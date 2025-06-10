#include <stdint-gcc.h>
#include <limits.h>
#include <dumbstring.h>

#include <vga.h>
#include <dumbio.h>
#include <keyboard.h>

#include <pic.h>
#include <interrupts.h>
#include <serial.h>

#include <multiboot.h>
#include <pageframe.h>
#include <pagetable.h>
#include <kmalloc.h>

void test_printk();
void test_keyboard_polling();
void pf_alloc_test(multiboot_fixed_header_t* mb_header);
void vp_alloc_test(multiboot_fixed_header_t* mb_header);
void kmalloc_test();

void kmain(multiboot_fixed_header_t* mb_header) {
  CLI();
    VGA_clear();

    TSS_setup();
    PIC_remap(M_PIC_OFFSET, S_PIC_OFFSET);
    setup_idt();
    SER_init();
    // PS2_setup();
  STI();
  
  VGA_clear();
  pf_alloc_init(mb_header);
  setup_kernel_page_table();
  kmalloc_test();

  while(1);
}

void kmalloc_test() {
  kmalloc_init();
  uint8_t* addr = kmalloc(8);
  printk("%p\n", addr);
  const char* bruh = "BRUH.";
  memcpy(addr, bruh, 6);
  printk("%s\n", addr);
  
  uint8_t* addr1 = kmalloc(8);
  printk("%p\n", addr1);
  kfree(addr);
  uint8_t* addr2 = kmalloc(8);
  printk("%p\n", addr2);

  for(int i = 0; i < NUM_POOLS + 2; i++) {
    uint64_t pool_size = (1 << (5+i));
    uint64_t num_allocs = i < NUM_POOLS ? (PAGE_SIZE / pool_size) * 3 / 2 : 1;
    char** addresses = kmalloc(sizeof(char*) * num_allocs);
    const char* bruhbruh = "bruh:)";
    for (int j = 0; j < num_allocs; j++) {
      addresses[j] = kmalloc(pool_size - sizeof(kmalloc_chunk_header_t));
      printk("addresses[%d]: %p\n", j, addresses[j]);
      memcpy(addresses[j], bruhbruh, 7);
    }
    for (int j = 0; j < num_allocs; j++) {
      if (j % 2 == 0) {
        printk("%d: %s\n", j, addresses[j]);
      }
      else {
        kfree(addresses[j]);
      }
    }
    for (int j = 0; j < num_allocs; j++) {
      if (j % 2 == 0) {
        kfree(addresses[j]);
      }
    }

    kfree(addresses);
  }
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

void vp_alloc_test(multiboot_fixed_header_t* mb_header) {

  pf_alloc_init(mb_header);
  setup_kernel_page_table();

  void* vp;
  vp = MMU_alloc_page();
  *(uintptr_t*)vp = (uintptr_t)vp;
  MMU_free_page(vp);
  printk("%lx\n", *(uintptr_t*)vp);

  pf_alloc_init(mb_header);
  setup_kernel_page_table();

  for(int i = 1;;i++) {
    for (int j = 0; j < (1 << i); j++) {
      vp = MMU_alloc_page();
      if (j % 2 == 0) *(uintptr_t*)vp = (uintptr_t)vp;
    }
    MMU_free_page(vp);
  }
  
  pf_alloc_init(mb_header);
  setup_kernel_page_table();

  for(;;) {
    vp = MMU_alloc_page();
    if ((uintptr_t)vp >= KHEAP_START + KHEAP_START) {
      printk("%p\n", vp);
    }
    MMU_free_page(vp);

  }

}