all:

ifeq "" "$(filter build,$(notdir $(CURDIR)))"
include target.mk
else

PLATFORM=i586

CC := i586-elf-gcc
CFLAGS := -I $(SRCDIR)/include -I $(SRCDIR)/arch/$(PLATFORM)/include \
					-gstabs+ -DDEBUG -fno-builtin -nostdlib -nostartfiles -nodefaultlibs -ffreestanding

CXX := i586-elf-g++
CXXFLAGS := -I $(SRCDIR)/include -I $(SRCDIR)/arch/$(PLATFORM)/include \
						-gstabs+ -DDEBUG -Wall -Wextra -Werror -nostdlib \
						-fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions -ffreestanding \
						-std=c++11 -fno-rtti -fno-stack-protector

AS := i586-elf-as
ASFLAGS := -gstabs

LINK.o := i586-elf-ld


build_dir := build
sources := loader.s main_entry.cc
           
TSTDIR := $(SRCDIR)/tests
test_sources := BitmapTest.cpp test_main.cpp

objects := $(subst .s,.o,$(subst .c,.o,$(subst .cc,.o,$(sources))))
dependencies := $(subst .c,.d,$(filter %.c,$(sources)))
dependencies += $(subst .cc,.d,$(filter %.cc,$(sources)))

vpath %.c $(SRCDIR)/kernel $(SRCDIR)/arch/$(PLATFORM)
vpath %.cc $(SRCDIR)/kernel $(SRCDIR)/arch/$(PLATFORM)
vpath %.s $(SRCDIR)/arch/$(PLATFORM)
vpath % $(SRCDIR)/kernel $(SRCDIR)/arch/$(PLATFORM) $(SRCDIR)/include $(SRCDIR)/arch/$(PLATFORM)/include
vpath %.h $(SRCDIR)/kernel $(SRCDIR)/arch/$(PLATFORM) $(SRCDIR)/include $(SRCDIR)/arch/$(PLATFORM)/include
vpath %.ld $(SRCDIR)/arch/$(PLATFORM)

all : kernel.elf bootable.iso documentation

ifneq "$(MAKECMDGOALS)" "clean"
  include $(dependencies)
endif

%.d: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -M $< | \
	sed 's,\($*\.o\) *:,\1 $@: ,' > $@.tmp
	mv $@.tmp $@

%.d: %.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -M $< | \
	sed 's,\($*\.o\) *:,\1 $@: ,' > $@.tmp
	mv $@.tmp $@


kernel.elf: linker.ld $(objects)
	$(LINK.o) -T $^ $(LOADLIBES) $(LDLIBS) -o $@
	
kernel.sym: kernel.elf
	objcopy --only-keep-debug $^ $@
	objcopy --strip-debug $^

bootable.iso: kernel.elf $(SRCDIR)/grub/menu.lst
	genisoimage -graft-points -R -b boot/grub/stage2_eltorito -no-emul-boot \
		-boot-load-size 4 -boot-info-table -o $@ \
		boot/kernel.elf=kernel.elf \
		boot/grub/menu.lst=$(SRCDIR)/grub/menu.lst \
		boot/grub/stage2_eltorito=../thirdparty/grub/stage2_eltorito

.PHONY: documentation
documentation:
	doxygen $(SRCDIR)/../docs/Doxyfile

endif

