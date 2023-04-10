include common/Makefile

SUB_DIRS:=boot hv

.PHONY: all
all: $(SUB_DIRS)

.PHONY: $(SUB_DIRS)
$(SUB_DIRS):
	$(MAKE) -C $@

OVMF_PATH?=$(shell ./scripts/select_qemu_firmware.sh)
ifdef QEMU_GDB
	QEMU_ARGS:=-S -s
endif
.PHONY: run
run: $(SUB_DIRS)
	qemu-system-x86_64 -enable-kvm \
		-cpu host \
		-drive file=fat:rw:boot/hda/,format=raw \
		-drive file="$(OVMF_PATH)",if=pflash,format=raw,readonly=on \
		-m 512M \
		-serial stdio \
		$(QEMU_ARGS)

.PHONY: clean
clean:
	for d in $(SUB_DIRS); do $(MAKE) -C $$d clean; done

.PHONY: format
format:
	git ls-files '*.c' '*.h' | xargs clang-format -i
