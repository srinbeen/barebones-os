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
# BUILD_SUBDIRS:=$(foreach path,$(SUBDIRS),$(BUILD_DIR)/$(path))

SRC_FILES:=$(foreach subdir,$(SRC_SUBDIRS),$(wildcard $(subdir)/*.c) $(wildcard $(subdir)/*.asm))
BUILD_FILES:=$(patsubst $(SRC_DIR)%,$(BUILD_DIR)%,$(SRC_FILES))
BUILD_FILES:=$(foreach f,$(BUILD_FILES),$(basename $(f)).o)

# SRC_FILES_ASM:=$(foreach subdir,$(SRC_SUBDIRS),$(wildcard $(subdir)/*.asm))
# SRC_FILES_C:=$(foreach subdir,$(SRC_SUBDIRS),$(wildcard $(subdir)/*.c))

# BUILD_FILES_ASM:=$(patsubst $(SRC_DIR)%,$(BUILD_DIR)%,$(SRC_FILES_ASM))
# BUILD_FILES_ASM:=$(foreach f,$(BUILD_FILES_ASM),$(basename $(f)).o)

# BUILD_FILES_C:=$(patsubst $(SRC_DIR)%,$(BUILD_DIR)%,$(SRC_FILES_C))
# BUILD_FILES_C:=$(foreach f,$(BUILD_FILES_C),$(basename $(f)).o)

# $(info $(BUILD_FILES_C))

# define build_objs
# $(foreach i, $(shell seq 1 $(words $(BUILD_FILES))), \
# 	$(eval $(word $(i),$(BUILD_FILES)) : $(word $(i),$(SRC_FILES)); \
# 		@echo $$@ --> $$<))
# endef

# define build_objs
# $(foreach i, $(shell seq 1 $(words $(BUILD_FILES))), \
# 	$(word $(i),$(BUILD_FILES)) : $(word $(i),$(SRC_FILES)))
# endef

# define build_objs
# $(foreach i, $(shell seq 1 $(words $(BUILD_FILES))), \
# 	$(eval $(word $(i),$(BUILD_FILES)): $(word $(i),$(SRC_FILES));) \
# 	$(eval $(word $(i),$(BUILD_FILES)) : ; \
# 		@mkdir -p $(dir $$@) ; \
# 		$(if $(filter %.c, $$<), \
# 			@echo building $$@ with $$< as a C file, \
# 			@echo building $$@ with $$< as an ASM file)))
# endef

# $(CC) $(CCFLAGS) -c -o $$@ $$<, \
# $(AS) $(ASFLAGS) -o $$@ $$<)))

# BOOT_SUBDIR=boot
# BUILD_BOOT_SUBDIR=$(BUILD_DIR)/$(BOOT_SUBDIR)
# SRC_BOOT_SUBDIR=$(SRC_DIR)/$(BOOT_SUBDIR)

# SRCS_BOOT=$(wildcard $(SRC_BOOT_SUBDIR)/*.asm)
# OBJS_BOOT=$(foreach file,$(SRCS_BOOT),$(BUILD_BOOT_SUBDIR)/$(notdir $(basename $(file))).o)

# LIB_SUBDIR=lib
# BUILD_LIB_SUBDIR=$(BUILD_DIR)/$(LIB_SUBDIR)
# SRC_LIB_SUBDIR=$(SRC_DIR)/$(LIB_SUBDIR)

# SRCS_LIB=$(wildcard $(SRC_LIB_SUBDIR)/*.c)
# OBJS_LIB=$(foreach file,$(SRCS_LIB),$(BUILD_LIB_SUBDIR)/$(notdir $(basename $(file))).o)

# DRIVERS_SUBDIR=drivers
# BUILD_DRIVERS_SUBDIR=$(BUILD_DIR)/$(DRIVERS_SUBDIR)
# SRC_DRIVERS_SUBDIR=$(SRC_DIR)/$(DRIVERS_SUBDIR)

# SRCS_DRIVERS=$(wildcard $(SRC_DRIVERS_SUBDIR)/*.c)
# OBJS_DRIVERS=$(foreach file,$(SRCS_DRIVERS),$(BUILD_DRIVERS_SUBDIR)/$(notdir $(basename $(file))).o)

# OBJS=$(OBJS_BOOT) $(OBJS_LIB) $(OBJS_DRIVERS)

KERNEL_NAME:=kmain
KERNEL_SRC:=$(SRC_DIR)/$(KERNEL_NAME).c
KERNEL_OBJ:=$(BUILD_DIR)/$(KERNEL_NAME).o
KERNEL:=$(BUILD_DIR)/kernel.bin

LO_SH:=./find_loop_dev
DEV_LOOP=$(shell $(LO_SH))
DEV_LOOP_1=$(shell expr $(DEV_LOOP) + 1)

OSFILES:=/mnt/osfiles

all: $(BUILD_DIR) $(BUILD_FILES)
	@echo done
# qemu-system-x86_64 -s -drive format=raw,file=$<

$(BUILD_DIR):
	@mkdir -p $@

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

kernel: $(KERNEL)

# $(BUILD_BOOT_SUBDIR)/%.o : $(SRC_BOOT_SUBDIR)/%.asm $(BUILD_BOOT_SUBDIR)
# 	$(AS) $(ASFLAGS) -o $@ $<

# $(BUILD_LIB_SUBDIR)/%.o : $(SRC_LIB_SUBDIR)/%.c $(BUILD_LIB_SUBDIR)
# 	$(CC) $(CCFLAGS) -c -o $@ $<

# $(BUILD_DRIVERS_SUBDIR)/%.o : $(SRC_DRIVERS_SUBDIR)/%.c $(BUILD_DRIVERS_SUBDIR)
# 	$(CC) $(CCFLAGS) -c -o $@ $<

$(KERNEL_OBJ) : $(KERNEL_SRC) $(BUILD_DIR)
	$(CC) $(CCFLAGS) -c -o $@ $<

# $(BUILD_DIR): 
# 	mkdir -p $@
# $(BUILD_BOOT_SUBDIR): 
# 	mkdir -p $@
# $(BUILD_LIB_SUBDIR): 
# 	mkdir -p $@
# $(BUILD_DRIVERS_SUBDIR): 
# 	mkdir -p $@

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
