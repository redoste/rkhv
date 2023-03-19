CC=clang
ASM=nasm

CPPFLAGS_COMMON:=-DRKHV_HASH=\"$(shell git rev-parse --short HEAD 2>/dev/null || echo 0000000)\" -I../common/include/
CFLAGS_WARNS:=-Wall -Wextra -Wstrict-prototypes -Wpedantic -Wswitch-enum -Wvla -Werror -Wno-language-extension-token
AFLAGS_WARNS:=-Werror

CFLAGS_FREESTANDING:=-nostdlibinc -ffreestanding -mno-red-zone
LFLAGS_FREESTANDING:=-nostdlib -static

ifdef DEBUG
	CFLAGS_OPT:=-O0 -g
	LFLAGS_OPT:=-g
else
	CFLAGS_OPT:=-O2 -flto
	LFLAGS_OPT:=-flto
endif
