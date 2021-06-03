OBJS =                                       \
	start.o                              \
	main.o				     
# Try to infer the correct TOOLPREFIX if not set
ifndef TOOLPREFIX
TOOLPREFIX := $(shell if riscv64-unknown-linux-gnu-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
        then echo 'riscv64-unknown-linux-gnu-'; \
        elif riscv64-unknown-linux-gnu-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
        then echo 'riscv64-unknown-linux-gnu-'; \
        else echo "***" 1>&2; \
        echo "*** Error: Couldn't find an riscv64 version of GCC/binutils." 1>&2; \
        echo "*** To turn off this error, run 'gmake TOOLPREFIX= ...'." 1>&2; \
        echo "***" 1>&2; exit 1; fi)
endif

CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)as
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += -I.
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]no-pie'),)
CFLAGS += -fno-pie -no-pie
endif
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]nopie'),)
CFLAGS += -fno-pie -nopie
endif

LDFLAGS = -z max-page-size=4096

all: Image
Image: $(subst .c,.o,$(wildcard *.c)) $(subst .S,.o,$(wildcard *.S))
	$(LD) $(LDFLAGS) -T link.ld -o startup.elf $(OBJS)
	$(OBJCOPY) startup.elf --strip-all -O binary $@

# compile all .c and .S file to .o file
%.o: %.c %.s
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.d *.o startup.elf *.asm Image

asm: Image
	$(OBJDUMP) -S fw_bin/startup.elf > fw_bin/demo.asm
