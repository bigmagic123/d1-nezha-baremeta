ifeq ($(V), 1)
	Q :=
else
	Q := @
endif
CROSS_COMPILE=riscv64-unknown-linux-gnu-

STANDBY = sun20iw1p1
#makefile for standby.bin
STANDBY_COMPILE = y
SRAM_BASE_LOAD=0x21000
SRAM_BASE=0x20000

STANDBY_XN_FILES := standby.xn

INCLUDE = -Iinclude
INCLUDE += -Iclk/$(STANDBY)/
INCLUDE += -Idram/

KBUILD_CFLAGS := -g -c -nostdlib -DSRAM_BASE=$(SRAM_BASE) -fno-unwind-tables -fno-jump-tables -fno-asynchronous-unwind-tables -O2 -mcmodel=medany
LD_FLAGS = -static

standby-c := $(wildcard *.c dram/dram.c clk/$(STANDBY)/*.c libsrc/*.c power/$(STANDBY)/*.c \
		power/*.c driver/twi/*.c driver/pin/*.c driver/clk/*.c driver/uart/*.c)
standby-s := $(wildcard libsrc/*.S)
standby-s += sunxi_standby.S
standby-s += $(wildcard dram/$(STANDBY)/mctl_standby-sun8iw20.S)
standby-y := $(patsubst %.c,%.o,$(standby-c))
standby-y += $(patsubst %.S,%.o,$(standby-s))

standby.bin: standby.elf
	$(Q)echo " COPY      (standby)standby.bin"
	$(Q)$(CROSS_COMPILE)strip -s standby.elf -o $(OUT)

standby.elf:  $(standby-y) FORCE
	$(Q)echo " LD        (standby)standby.elf"
	$(Q)$(CROSS_COMPILE)ld -T $(STANDBY_XN_FILES) -Ttext=$(SRAM_BASE_LOAD) $(LD_FLAGS) -EL  $(standby-y) $(standby-lib) -o standby.elf -Map standby.map
	$(Q)$(CROSS_COMPILE)objdump -D standby.elf > standby.lst

%.o : %.c
	$(Q)echo " CC        (standby)$<"
	$(Q)$(CROSS_COMPILE)gcc $(KBUILD_CFLAGS) $(INCLUDE) $< -o $@

%.o : %.S
	$(Q)echo " AS        (standby)$<"
	$(Q)$(CROSS_COMPILE)gcc $(KBUILD_CFLAGS) $(INCLUDE) $< -o $@

clean:
	$(Q)rm -f $(standby-y) standby.code standby.elf *.lst *.map

.PHONY:
FORCE:
