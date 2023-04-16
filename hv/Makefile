include ../common/Makefile

HV_CC?=$(CC)
HV_ASM?=$(ASM)
HV_TARGET?=x86_64-elf

HV_CPPFLAGS?=$(CPPFLAGS_COMMON) -Iinclude/ -DRKHV_HV
HV_CFLAGS?=-target $(HV_TARGET) $(CFLAGS_FREESTANDING) -mcmodel=large $(CFLAGS_WARNS) $(CFLAGS_OPT)
HV_LFLAGS?=-target $(HV_TARGET) $(LFLAGS_FREESTANDING) -fuse-ld=lld -T rkhv.ld $(LFLAGS_OPT)
HV_AFLAGS?=-f elf64 $(AFLAGS_WARNS)

SRCS_C:=src/main.c\
        src/interrupts.c\
        src/memory_management/arena.c\
        src/memory_management/memory_management.c\
        src/memory_management/vmm_vmx_pages.c\
        src/panic.c\
        src/segments.c\
        src/serial.c\
        src/stdio.c\
        src/string.c\
        src/vmm/vm_emulated_instructions.c\
        src/vmm/vm_ioports.c\
        src/vmm/vm_manager.c\
        src/vmm/vmx_ept.c\
        src/vmm/vmx_init.c\
        src/vmm/vmx_instructions.c\
        src/vmm/vmx_vmcs.c\
        src/vmm/vmx_vmexit.c
SRCS_A:=src/interrupts.asm\
        src/segments.asm\
        src/vmm/guest_hello_world.asm\
        src/vmm/vmx_instructions.asm\
        src/vmm/vmx_vmexit.asm

OBJ_DIR:=obj
OBJS_C:=$(addprefix $(OBJ_DIR)/,$(SRCS_C:%.c=%.o))
OBJS_A:=$(addprefix $(OBJ_DIR)/,$(SRCS_A:%.asm=%_asm.o))
OBJS:=$(OBJS_C) $(OBJS_A)

OUT_EXEC:=$(OBJ_DIR)/rkhv.elf

OUT_DIR:=../boot/hda/rkhv
OUT_RX:=$(OUT_DIR)/rkhv.rx
OUT_RO:=$(OUT_DIR)/rkhv.ro
OUT_RW:=$(OUT_DIR)/rkhv.rw

.PHONY: all
all: $(OUT_RX) $(OUT_RO) $(OUT_RW)

$(OUT_RX): $(OUT_EXEC)
	@mkdir -p $(dir $@)
	objcopy -O binary --only-section=.text $< $@

$(OUT_RO): $(OUT_EXEC)
	@mkdir -p $(dir $@)
	objcopy -O binary --only-section=.rodata $< $@

$(OUT_RW): $(OUT_EXEC)
	@mkdir -p $(dir $@)
	objcopy -O binary --only-section=.data $< $@

$(OUT_EXEC): $(OBJS)
	@mkdir -p $(dir $@)
	$(HV_CC) $(HV_LFLAGS) $^ -o $@

$(OBJS_C): $(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(HV_CC) -MMD -MT $@ $(HV_CPPFLAGS) $(HV_CFLAGS) -c $< -o $@

$(OBJS_A): $(OBJ_DIR)/%_asm.o: %.asm
	@mkdir -p $(dir $@)
	$(HV_ASM) -MD $(@:%.o=%.d) -MT $@ $(HV_AFLAGS) $< -o $@

-include $(OBJS:%.o=%.d)

.PHONY: clean
clean:
	-rm -rv $(OBJ_DIR) $(OUT_DIR)
