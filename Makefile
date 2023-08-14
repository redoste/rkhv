include common/Makefile

SUB_DIRS:=boot hv

.PHONY: all
all: $(SUB_DIRS)

.PHONY: $(SUB_DIRS)
$(SUB_DIRS):
	$(MAKE) -C $@

.PHONY: img
img: $(SUB_DIRS)
	$(MAKE) -C boot hda.img
.PHONY: vdi
vdi: $(SUB_DIRS)
	$(MAKE) -C boot hda.vdi

OVMF_PATH?=$(shell ./scripts/select_qemu_firmware.sh)
QEMU_RAM?=512M
ifdef QEMU_GDB
	QEMU_ARGS:=-S -s
endif
ifdef QEMU_CPU_HOST
	QEMU_CPU?=host
else
	QEMU_CPU?=qemu64,invpcid,vmx,vmx-entry-ia32e-mode,vmx-entry-load-efer,vmx-ept,vmx-exit-load-efer,vmx-exit-save-efer,vmx-hlt-exit,vmx-invpcid-exit,vmx-io-exit,vmx-msr-bitmap,vmx-page-walk-4,vmx-secondary-ctls,vmx-unrestricted-guest,vmx-xsaves,xsave,xsaves
endif
.PHONY: run
run: $(SUB_DIRS)
	qemu-system-x86_64 -enable-kvm \
		-cpu "$(QEMU_CPU)" \
		-drive file=fat:rw:boot/hda/,format=raw \
		-drive file="$(OVMF_PATH)",if=pflash,format=raw,readonly=on \
		-m "$(QEMU_RAM)" \
		-serial stdio \
		$(QEMU_ARGS)

.PHONY: clean
clean:
	for d in $(SUB_DIRS); do $(MAKE) -C $$d clean; done

.PHONY: format
format:
	git ls-files '*.c' '*.h' | xargs clang-format -i
