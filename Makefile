include common/Makefile

SUB_DIRS:=boot hv
FORMATTED_DIRS:=$(SUB_DIRS) common

.PHONY: all
all: $(SUB_DIRS)

.PHONY: $(SUB_DIRS)
$(SUB_DIRS):
	$(MAKE) -C $@

OVMF_PATH?=/usr/share/edk2/x64/OVMF.fd
ifdef QEMU_GDB
	QEMU_ARGS:=-S -s
endif
.PHONY: run
run: $(SUB_DIRS)
	qemu-system-x86_64 -enable-kvm \
		-drive file=fat:rw:boot/hda/,format=raw \
		-bios $(OVMF_PATH) \
		-m 512M \
		-serial stdio \
		$(QEMU_ARGS)

.PHONY: clean
clean:
	for d in $(SUB_DIRS); do $(MAKE) -C $$d clean; done

.PHONY: format
format:
	find $(FORMATTED_DIRS) -iname *.h -o -iname *.c | xargs clang-format -i
