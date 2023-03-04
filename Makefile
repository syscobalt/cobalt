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

REPO_ROOT = .

include $(REPO_ROOT)/build-aux/arch.mk
include $(REPO_ROOT)/build-aux/paths.mk
include $(REPO_ROOT)/build-aux/toolchain.mk

KERNEL = $(BUILD_DIR)/kernel/kernel
INITRD = $(BUILD_DIR)/initrd.tar.xz
ISO = cobalt.iso
LICENSE = $(LICENSES_DIR)/cobalt/LICENSE

DXPORT = ./ports/dxport --host=$(ARCH)-cobalt --builddir=$(BUILD_DIR)/install-ports
DXPORT += --sysroot=$(SYSROOT)

all: libc kernel apps sh utils iso

apps: $(INCLUDE_DIR) $(LIB_DIR)
	$(MAKE) -C apps

kernel $(KERNEL): $(INCLUDE_DIR) $(LIB_DIR)
	$(MAKE) -C kernel

libc: $(INCLUDE_DIR)
	$(MAKE) -C libc

install-all: install-headers install-libc
install-all: install-apps install-sh install-utils

install-apps:
	$(MAKE) -C apps install

install-headers $(INCLUDE_DIR):
	$(MAKE) -C kernel install-headers
	$(MAKE) -C libc install-headers

install-libc: $(INCLUDE_DIR)
	$(MAKE) -C libc install-libs

$(LIB_DIR):
	$(MAKE) -C libc install-libs

install-ports $(DXPORT_DIR): $(INCLUDE_DIR) $(LIB_DIR)
ifneq ($(wildcard ./ports/dxport),)
	-$(DXPORT) install -k all
endif

install-sh: $(INCLUDE_DIR) $(LIB_DIR)
	$(MAKE) -C sh install

install-toolchain: install-headers
	SYSROOT=$(SYSROOT) $(REPO_ROOT)/build-aux/install-toolchain.sh

install-utils: $(INCLUDE_DIR) $(LIB_DIR)
	$(MAKE) -C utils install

iso: $(ISO)

$(ISO): $(KERNEL) $(INITRD)
	rm -rf $(BUILD_DIR)/isosrc
	mkdir -p $(BUILD_DIR)/isosrc/boot/grub
	cp -f build-aux/grub.cfg $(BUILD_DIR)/isosrc/boot/grub
	cp -f $(KERNEL) $(BUILD_DIR)/isosrc
	cp -f $(INITRD) $(BUILD_DIR)/isosrc
	$(MKRESCUE) -o $@ $(BUILD_DIR)/isosrc

$(INITRD): $(SYSROOT)
	cd $(SYSROOT) && tar cJf ../$(INITRD) --format=ustar *

qemu: $(ISO)
	qemu-system-$(BASE_ARCH) -cdrom $^ -m 1024M -cpu host -enable-kvm

sh: $(INCLUDE_DIR) $(LIB_DIR)
	$(MAKE) -C sh

utils: $(INCLUDE_DIR) $(LIB_DIR)
	$(MAKE) -C utils

$(SYSROOT): $(INCLUDE_DIR) $(LIB_DIR) $(BIN_DIR) $(SYSROOT)/usr $(LICENSE)
$(SYSROOT): $(SYSROOT)/share/fonts/vgafont $(SYSROOT)/home/user

$(BIN_DIR):
	$(MAKE) -C apps install
	$(MAKE) -C sh install
	$(MAKE) -C utils install

$(LICENSE): LICENSE
	@mkdir -p $(LICENSES_DIR)/cobalt
	cp -f LICENSE $@

$(SYSROOT)/usr:
	ln -s . $@

$(SYSROOT)/share/fonts/vgafont: kernel/vgafont
	@mkdir -p $(dir $@)
	cp -f kernel/vgafont $@

$(SYSROOT)/home/user:
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(ISO)

distclean:
	rm -rf build sysroot
	rm -f *.iso

.PHONY: all apps kernel libc install-all install-apps
.PHONY: install-headers install-libc install-ports install-sh
.PHONY: install-toolchain install-utils iso qemu sh utils clean distclean
