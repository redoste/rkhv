include ../common/Makefile

BOOT_CC?=$(CC)
BOOT_ASM?=$(ASM)
BOOT_TARGET?=x86_64-unknown-windows

GNU_EFI_INC:=../vendor/gnu-efi/inc
BOOT_CPPFLAGS?=-I $(GNU_EFI_INC) -isystem $(GNU_EFI_INC) $(CPPFLAGS_COMMON) -DRKHV_BOOT
BOOT_CFLAGS?=-target $(BOOT_TARGET) $(CFLAGS_FREESTANDING) $(CFLAGS_WARNS) $(CFLAGS_OPT)
BOOT_LFLAGS?=-target $(BOOT_TARGET) $(LFLAGS_FREESTANDING) -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld-link $(LFLAGS_OPT)
BOOT_AFLAGS?=-f win64 $(AFLAGS_WARNS)

SRCS_C:=src/main.c\
        src/fs.c\
        src/paging.c\
        src/stdio.c\
        src/string.c
SRCS_A:=src/chainload.asm

OBJ_DIR:=obj
OBJS_C:=$(addprefix $(OBJ_DIR)/,$(SRCS_C:%.c=%.o))
OBJS_A:=$(addprefix $(OBJ_DIR)/,$(SRCS_A:%.asm=%_asm.o))
OBJS:=$(OBJS_C) $(OBJS_A)

OUT_DIR:=hda/EFI/BOOT
OUT_EXEC:=$(OUT_DIR)/BOOTX64.EFI

DISK_FOLDER:=hda/
DISK_DEPS:=$(OUT_EXEC) hda/rkhv/*
DISK_IMG:=hda.img
DISK_VDI:=hda.vdi

.PHONY: all
all: $(OUT_EXEC)

$(OUT_EXEC): $(OBJS)
	@mkdir -p $(dir $@)
	$(BOOT_CC) $(BOOT_LFLAGS) $^ -o $@

$(OBJS_C): $(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(BOOT_CC) -MMD -MT $@ $(BOOT_CPPFLAGS) $(BOOT_CFLAGS) -c $< -o $@

$(OBJS_A): $(OBJ_DIR)/%_asm.o: %.asm
	@mkdir -p $(dir $@)
	$(BOOT_ASM) -MD $(@:%.o=%.d) -MT $@ $(BOOT_AFLAGS) $< -o $@

-include $(OBJS:%.o=%.d)

$(DISK_IMG): $(DISK_FOLDER) $(DISK_DEPS)
	dd if=/dev/zero bs=1024 count=16384 of=$@ status=none
	mformat -i $@ -v rkhv ::.
	mcopy -i $@ -s $</* ::.

# UUID v5 generated using the following parameters :
# $ python3 -c 'import uuid ; print(uuid.uuid5(uuid.UUID(bytes=b"\x00"*16), "rkhv"))'
$(DISK_VDI): $(DISK_IMG)
	-rm $@
	VBoxManage convertfromraw --uuid="d67954b2-3a7e-589b-bdba-f7b7bcba1c60" $< $@

.PHONY: clean
clean:
	-rm -rv $(OBJ_DIR) $(OUT_DIR) $(DISK_IMG) $(DISK_VDI)
