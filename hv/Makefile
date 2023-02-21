include ../common/Makefile

HV_CC?=$(CC)
HV_TARGET?=x86_64-elf

HV_CPPFLAGS?=$(CPPFLAGS_COMMON) -DRKHV_HV
HV_CFLAGS?=-target $(HV_TARGET) $(CFLAGS_FREESTANDING) -mcmodel=large $(CFLAGS_WARNS) $(CFLAGS_OPT)
HV_LFLAGS?=-target $(HV_TARGET) $(LFLAGS_FREESTANDING) -fuse-ld=lld -T rkhv.ld $(LFLAGS_OPT)

SRCS:=src/main.c\
      src/serial.c\
      src/stdio.c\
      src/string.c

OBJ_DIR:=obj
OBJS:=$(addprefix $(OBJ_DIR)/,$(SRCS:%.c=%.o))

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

$(OBJS): $(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(HV_CC) -MMD -MT $@ $(HV_CPPFLAGS) $(HV_CFLAGS) -c $< -o $@

-include $(OBJS:%.o=%.d)

.PHONY: clean
clean:
	-rm -rv $(OBJ_DIR) $(OUT_DIR)
