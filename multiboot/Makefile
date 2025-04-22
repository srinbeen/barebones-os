# use -f for elf64
AS=nasm
LD=x86_64-elf-ld
CC=x86_64-elf-gcc

SRC_DIR=src
BUILD_DIR=build
LINKERFILE=linker.ld
IMG_DIR=img
IMG_FILE=os.img

# -f elf64 : elf64 format
ASFLAGS=-f elf64
CCFLAGS=-Wall -Werror -g -std=c99 -ffreestanding -mno-red-zone -I/home/srini/cpe454/multiboot/include

# -n --nmagic : prevents page alignment so that grub can find mulitboot header
LDFLAGS=-n



BOOT_SUBDIR=boot
BUILD_BOOT_SUBDIR=$(BUILD_DIR)/$(BOOT_SUBDIR)
SRC_BOOT_SUBDIR=$(SRC_DIR)/$(BOOT_SUBDIR)

SRCS_BOOT=$(wildcard $(SRC_BOOT_SUBDIR)/*.asm)
OBJS_BOOT=$(foreach file,$(SRCS_BOOT),$(BUILD_BOOT_SUBDIR)/$(notdir $(basename $(file))).o)

LIB_SUBDIR=lib
BUILD_LIB_SUBDIR=$(BUILD_DIR)/$(LIB_SUBDIR)
SRC_LIB_SUBDIR=$(SRC_DIR)/$(LIB_SUBDIR)

SRCS_LIB=$(wildcard $(SRC_LIB_SUBDIR)/*.c)
OBJS_LIB=$(foreach file,$(SRCS_LIB),$(BUILD_LIB_SUBDIR)/$(notdir $(basename $(file))).o)

DRIVERS_SUBDIR=drivers
BUILD_DRIVERS_SUBDIR=$(BUILD_DIR)/$(DRIVERS_SUBDIR)
SRC_DRIVERS_SUBDIR=$(SRC_DIR)/$(DRIVERS_SUBDIR)

SRCS_DRIVERS=$(wildcard $(SRC_DRIVERS_SUBDIR)/*.c)
OBJS_DRIVERS=$(foreach file,$(SRCS_DRIVERS),$(BUILD_DRIVERS_SUBDIR)/$(notdir $(basename $(file))).o)

OBJS=$(OBJS_BOOT) $(OBJS_LIB) $(OBJS_DRIVERS)

KERNEL_NAME=kmain
KERNEL_SRC=$(SRC_DIR)/$(KERNEL_NAME).c
KERNEL_OBJ=$(BUILD_DIR)/$(KERNEL_NAME).o
KERNEL=$(BUILD_DIR)/kernel.bin

LO_SH=./find_loop_dev
DEV_LOOP=$(shell $(LO_SH))
DEV_LOOP_1=$(shell expr $(DEV_LOOP) + 1)

OSFILES=/mnt/osfiles

all: $(IMG_FILE)
	qemu-system-x86_64 -drive format=raw,file=$<

kernel: $(KERNEL)

$(BUILD_BOOT_SUBDIR)/%.o : $(SRC_BOOT_SUBDIR)/%.asm $(BUILD_BOOT_SUBDIR)
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_LIB_SUBDIR)/%.o : $(SRC_LIB_SUBDIR)/%.c $(BUILD_LIB_SUBDIR)
	$(CC) $(CCFLAGS) -c -o $@ $<

$(BUILD_DRIVERS_SUBDIR)/%.o : $(SRC_DRIVERS_SUBDIR)/%.c $(BUILD_DRIVERS_SUBDIR)
	$(CC) $(CCFLAGS) -c -o $@ $<

$(KERNEL_OBJ) : $(KERNEL_SRC) $(BUILD_DIR)
	$(CC) $(CCFLAGS) -c -o $@ $<

$(BUILD_DIR): 
	mkdir -p $@
$(BUILD_BOOT_SUBDIR): 
	mkdir -p $@
$(BUILD_LIB_SUBDIR): 
	mkdir -p $@
$(BUILD_DRIVERS_SUBDIR): 
	mkdir -p $@

# Link obj files and get kernel
$(KERNEL): $(BUILD_DIR) $(OBJS) $(KERNEL_OBJ) $(LINKERFILE)
	$(LD) $(LDFLAGS) -o $@ -T $(LINKERFILE) $(OBJS) $(KERNEL_OBJ)

# Make img directory to copy to OSFILES
$(IMG_DIR): $(KERNEL) $(SRC_DIR)/grub.cfg
	mkdir -p $@
	mkdir -p $@/boot
	mkdir -p $@/boot/grub

	cp $(KERNEL) $@/boot
	cp $(SRC_DIR)/grub.cfg $@/boot/grub

$(IMG_FILE): $(IMG_DIR) $(LO_SH)
# Generate MBR on IMG
	dd if=/dev/zero of=$@ bs=512 count=32768
	parted $@ mklabel msdos
	parted $@ mkpart primary ext2 2048s 30720s
	parted $@ set 1 boot on

# Mount IMG to loopback devices
	sudo losetup /dev/loop$(DEV_LOOP) $@
	sudo losetup /dev/loop$(DEV_LOOP_1) $@ -o 1048576
	sudo mkfs.ext2 /dev/loop$(DEV_LOOP_1)

# Mount lo to OSFILES
	sudo mkdir -p $(OSFILES)
	sudo mount /dev/loop$(DEV_LOOP_1) $(OSFILES)

# Install Grub on IMG (FSBL, SSBL)
	sudo grub-install \
	--root-directory=$(OSFILES) \
	--no-floppy --target=i386-pc \
	--modules="normal part_msdos ext2 multiboot" \
	/dev/loop$(DEV_LOOP)

# Copy disk structure (kernel, grub.cfg) to OSFILES
	sudo cp -r $(IMG_DIR)/* $(OSFILES)
	sudo umount $(OSFILES)

# Unmount
	sudo losetup -d /dev/loop$(DEV_LOOP)
	sudo losetup -d /dev/loop$(DEV_LOOP_1)



clean:
	rm -rf $(BUILD_DIR) $(KERNEL) $(IMG_DIR) $(IMG_FILE)

# Phony targets
.PHONY: all clean
