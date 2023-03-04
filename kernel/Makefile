#
#                                                                                      
#      ,ad8888ba,    ,ad8888ba,    88888888ba         db         88      888888888888  
#     d8"'    `"8b  d8"'    `"8b   88      "8b       d88b        88           88       
#    d8'           d8'        `8b  88      ,8P      d8'`8b       88           88       
#    88            88          88  88aaaaaa8P'     d8'  `8b      88           88       
#    88            88          88  88""""""8b,    d8YaaaaY8b     88           88       
#    Y8,           Y8,        ,8P  88      `8b   d8""""""""8b    88           88       
#     Y8a.    .a8P  Y8a.    .a8P   88      a8P  d8'        `8b   88           88       
#      `"Y8888Y"'    `"Y8888Y"'    88888888P"  d8'          `8b  88888888888  88       
#  Cobalt is a UNIX-like operating system forked from Dennis Wölfing's Dennix operating
#  system, which can be found at https://github.com/dennis95/dennix. Cobalt is licensed
#  under the ISC license, which can be found at the file called LICENSE at the root
#  directory of the project.
#

REPO_ROOT = ..

include $(REPO_ROOT)/build-aux/arch.mk
include $(REPO_ROOT)/build-aux/paths.mk
include $(REPO_ROOT)/build-aux/toolchain.mk
include $(REPO_ROOT)/build-aux/version.mk

BUILD = $(BUILD_DIR)/kernel
LDSCRIPT = $(BUILD)/linker.ld

CXXFLAGS ?= -O2 -g
CXXFLAGS += --sysroot=$(SYSROOT) -std=gnu++14 -ffreestanding -fno-exceptions
CXXFLAGS += -fno-rtti -fcheck-new -nostdinc++ -fstack-protector-strong
CXXFLAGS += -Wall -Wextra -Wno-missing-field-initializers
CPPFLAGS += -I include -DCOBALT_VERSION=\"$(VERSION)\"
CPPFLAGS += -D__is_cobalt_kernel -D_COBALT_SOURCE
LDFLAGS += --sysroot=$(SYSROOT) -T $(LDSCRIPT) -ffreestanding -nostdlib
LIBS += -lk -lgcc

OBJ = \
	acpi.o \
	addressspace.o \
	ahci.o \
	ata.o \
	bga.o \
	blockcache.o \
	circularbuffer.o \
	clock.o \
	conf.o \
	console.o \
	cxx.o \
	devices.o \
	directory.o \
	display.o \
	ext234fs.o \
	ext234vnode.o \
	file.o \
	filedescription.o \
	hpet.o \
	initrd.o \
	kernel.o \
	keyboard.o \
	kthread.o \
	libk.o \
	log.o \
	memorysegment.o \
	mouse.o \
	panic.o \
	partition.o \
	pci.o \
	physicalmemory.o \
	pipe.o \
	pit.o \
	process.o \
	ps2.o \
	ps2keyboard.o \
	ps2mouse.o \
	pseudoterminal.o \
	random.o \
	refcount.o \
	rtc.o \
	signal.o \
	streamsocket.o \
	symlink.o \
	syscall.o \
	terminal.o \
	thread.o \
	virtualbox.o \
	vnode.o \
	worker.o

all: $(BUILD)/kernel

include $(ARCH).mk

OBJ := $(addprefix $(BUILD)/, $(OBJ))
-include $(OBJ:.o=.d)

install-headers:
	@mkdir -p $(INCLUDE_DIR)
	cp -rf --preserve=timestamp include/. $(INCLUDE_DIR)
	touch $(SYSROOT)

$(BUILD)/kernel: $(OBJ) $(LDSCRIPT)
	$(CXX) $(LDFLAGS) -o $@ $(OBJ) $(LIBS)

$(LDSCRIPT): linker.ld.in
	@mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) -P -xc -o $@ $<

$(BUILD)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MD -MP -c -o $@ $<

$(BUILD)/%.o: $(BUILD)/%.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MD -MP -c -o $@ $<

$(BUILD)/%.o: src/%.S
	@mkdir -p $(dir $@)
	$(CXX) $(ASFLAGS) $(CPPFLAGS) -MD -MP -c -o $@ $<

clean:
	rm -rf $(BUILD)

.PHONY: all clean
