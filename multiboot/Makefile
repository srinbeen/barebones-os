# use -f for elf64
AS:=nasm
LD:=x86_64-elf-ld
CC:=x86_64-elf-gcc

SRC_DIR:=src
BUILD_DIR:=build
LINKERFILE:=linker.ld
IMG_DIR:=img
IMG_FILE:=os.img

# -f elf64 : elf64 format
ASFLAGS:=-f elf64
CCFLAGS:=-Wall -Werror -g -std=c99 -ffreestanding -mno-red-zone -I/home/srini/cpe454/multiboot/include

# -n --nmagic : prevents page alignment so that grub can find mulitboot header
LDFLAGS:=-n

SUBDIRS:=boot driver lib interrupt
SRC_SUBDIRS:=$(foreach path,$(SUBDIRS),$(SRC_DIR)/$(path))

SRC_FILES:=$(foreach subdir,$(SRC_SUBDIRS),$(wildcard $(subdir)/*.c) $(wildcard $(subdir)/*.asm))
BUILD_FILES:=$(patsubst $(SRC_DIR)%,$(BUILD_DIR)%,$(SRC_FILES))
BUILD_FILES:=$(foreach f,$(BUILD_FILES),$(basename $(f)).o)

KERNEL_NAME:=kmain
KERNEL_SRC:=$(SRC_DIR)/$(KERNEL_NAME).c
KERNEL_OBJ:=$(BUILD_DIR)/$(KERNEL_NAME).o
KERNEL:=$(BUILD_DIR)/kernel.bin

CFG_FILE:=$(SRC_DIR)/grub.cfg

LO_SH:=./find_loop_dev
DEV_LOOP=$(shell $(LO_SH))
DEV_LOOP_1=$(shell expr $(DEV_LOOP) + 1)

OSFILES:=/mnt/osfiles

all: $(IMG_FILE)
	@echo done.. booting qemu
	qemu-system-x86_64 -drive format=raw,file=$<

debug: $(IMG_FILE)
	@echo done.. booting qemu
	qemu-system-x86_64 -s -S -drive format=raw,file=$<

kernel: $(KERNEL)

# Link obj files and get kernel
$(KERNEL): $(BUILD_FILES) $(KERNEL_OBJ) $(LINKERFILE)
	$(LD) $(LDFLAGS) -o $@ -T $(LINKERFILE) $(BUILD_FILES) $(KERNEL_OBJ)

$(KERNEL_OBJ) : $(KERNEL_SRC)
	$(CC) $(CCFLAGS) -c -o $@ $<

build: $(BUILD_FILES)

define build_objs
$(eval $(1) : $(2) ; \
	@mkdir -p $(dir $(1)) ; \
	$(if $(filter %.c,$(2)),\
		$(CC) $(CCFLAGS) -c -o $(1) $(2),\
		$(AS) $(ASFLAGS) -o $(1) $(2)))
endef

$(foreach i,$(shell seq 1 $(words $(BUILD_FILES))), \
	$(call build_objs,\
		$(word $(i),$(BUILD_FILES)),\
		$(word $(i),$(SRC_FILES))))

# Make img directory to copy to OSFILES
$(IMG_DIR): $(KERNEL) $(CFG_FILE)
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
