include ../common/Makefile

BOOT_CC?=$(CC)
BOOT_ASM?=$(ASM)
BOOT_TARGET?=x86_64-unknown-windows

GNU_EFI_INC:=../vendor/gnu-efi/inc
BOOT_CPPFLAGS?=-I $(GNU_EFI_INC) -isystem $(GNU_EFI_INC) $(CPPFLAGS_COMMON) -DRKHV_BOOT
BOOT_CFLAGS?=-target $(BOOT_TARGET) $(CFLAGS_FREESTANDING) -mstack-probe-size=1048576 $(CFLAGS_WARNS) $(CFLAGS_OPT)
BOOT_LFLAGS?=-target $(BOOT_TARGET) $(LFLAGS_FREESTANDING) -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld-link $(LFLAGS_OPT)
BOOT_AFLAGS?=-f win64 $(AFLAGS_WARNS)

SRCS_C:=src/main.c\
        src/fs.c\
        src/mem.c\
        src/paging.c\
        src/stdio.c\
        src/string.c
SRCS_A:=src/chainload.asm

OBJ_DIR:=obj
OBJS_C:=$(addprefix $(OBJ_DIR)/,$(SRCS_C:%.c=%.o))
OBJS_A:=$(addprefix $(OBJ_DIR)/,$(SRCS_A:%.asm=%.o))
OBJS:=$(OBJS_C) $(OBJS_A)

OUT_DIR:=hda/EFI/BOOT
OUT_EXEC:=$(OUT_DIR)/BOOTX64.EFI

.PHONY: all
all: $(OUT_EXEC)

$(OUT_EXEC): $(OBJS)
	@mkdir -p $(dir $@)
	$(BOOT_CC) $(BOOT_LFLAGS) $^ -o $@

$(OBJS_C): $(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(BOOT_CC) -MMD -MT $@ $(BOOT_CPPFLAGS) $(BOOT_CFLAGS) -c $< -o $@

$(OBJS_A): $(OBJ_DIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(BOOT_ASM) -MD $(@:%.o=%.d) -MT $@ $(BOOT_AFLAGS) $< -o $@

-include $(OBJS:%.o=%.d)

.PHONY: clean
clean:
	-rm -rv $(OBJ_DIR) $(OUT_DIR)
