# xv6 Makefile

# Cross-compiler prefix
TOOLPREFIX := $(shell if riscv64-unknown-elf-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-unknown-elf-'; \
	elif riscv64-linux-gnu-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-linux-gnu-'; \
	elif riscv64-unknown-linux-gnu-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-unknown-linux-gnu-'; \
	else echo "***" 1>&2; \
	echo "*** Error: Couldn't find a riscv64 version of GCC/binutils." 1>&2; \
	echo "*** To turn off this error, run 'gmake TOOLPREFIX= ...'." 1>&2; \
	echo "***" 1>&2; exit 1; fi)

CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)as
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb -gdwarf-2
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += -I.
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

# Disable PIE when possible (for Ubuntu 16.10 toolchain)
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '--enable-default-pie'),)
CFLAGS += -fno-pie -no-pie
endif
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '--enable-default-ssp'),)
CFLAGS += -fno-stack-protector
endif

LDFLAGS = -z max-page-size=4096

# Kernel object files
KERNEL_OBJS = \
  kernel/entry.o \
  kernel/start.o \
  kernel/console.o \
  kernel/printf.o \
  kernel/uart.o \
  kernel/kalloc.o \
  kernel/spinlock.o \
  kernel/string.o \
  kernel/main.o \
  kernel/vm.o \
  kernel/proc.o \
  kernel/swtch.o \
  kernel/trap.o \
  kernel/syscall.o \
  kernel/sysproc.o \
  kernel/bio.o \
  kernel/fs.o \
  kernel/log.o \
  kernel/sleeplock.o \
  kernel/file.o \
  kernel/pipe.o \
  kernel/exec.o \
  kernel/sysfile.o \
  kernel/kernelvec.o \
  kernel/plic.o \
  kernel/virtio_disk.o

QEMU = qemu-system-riscv64

ifndef CPUS
CPUS := 3
endif

all: kernel/kernel

# Compile kernel
kernel/kernel: $(KERNEL_OBJS) kernel/kernel.ld
	$(LD) $(LDFLAGS) -T kernel/kernel.ld -o kernel/kernel $(KERNEL_OBJS) 
	$(OBJDUMP) -S kernel/kernel > kernel/kernel.asm
	$(OBJDUMP) -t kernel/kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > kernel/kernel.sym

kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/%.o: kernel/%.S
	$(CC) $(CFLAGS) -c -o $@ $<

clean: 
	rm -f *.tex *.dvi *.idx *.aux *.log *.ind *.ilg \
	kernel/*.o kernel/*.d kernel/kernel kernel/kernel.asm kernel/kernel.sym

# Try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
# QEMU's gdb stub command line changed in 0.11
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)

QEMUOPTS = -machine virt -bios none -kernel kernel/kernel -m 128M -smp $(CPUS) -nographic
QEMUOPTS += -global virtio-mmio.force-legacy=false
QEMUOPTS += -drive file=fs.img,if=none,format=raw,id=x0
QEMUOPTS += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

qemu: kernel/kernel
	$(QEMU) $(QEMUOPTS)

.gdbinit: .gdbinit.tmpl-riscv
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

qemu-gdb: kernel/kernel .gdbinit fs.img
	@echo "*** Now run 'gdb' in another window." 1>&2
	$(QEMU) $(QEMUOPTS) -S $(QEMUGDB)

# Create file system image placeholder
fs.img:
	dd if=/dev/zero of=fs.img bs=1M count=1

-include kernel/*.d