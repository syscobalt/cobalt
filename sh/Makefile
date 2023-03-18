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
#  system, which can be found at https://github.com/dennis95/cobalt. Cobalt is licensed
#  under the ISC license, which can be found at the file called LICENSE at the root
#  directory of the project.
#

REPO_ROOT = ..

include $(REPO_ROOT)/build-aux/arch.mk
include $(REPO_ROOT)/build-aux/paths.mk
include $(REPO_ROOT)/build-aux/toolchain.mk
include $(REPO_ROOT)/build-aux/version.mk

BUILD = $(BUILD_DIR)/sh

CFLAGS ?= -O2 -g
CFLAGS += --sysroot=$(SYSROOT) -fstack-protector-strong -Wall -Wextra
CPPFLAGS += -D_COBALT_SOURCE -DCOBALT_VERSION=\"$(VERSION)\"

OBJ = \
	builtins.o \
	execute.o \
	expand.o \
	interactive.o \
	match.o \
	parser.o \
	sh.o \
	stringbuffer.o \
	tokenizer.o \
	trap.o \
	variables.o

all: $(BUILD)/sh

OBJ := $(addprefix $(BUILD)/, $(OBJ))
-include $(OBJ:.o=.d)

install: $(BUILD)/sh
	@mkdir -p $(BIN_DIR)
	cp -f $^ $(BIN_DIR)
	touch $(SYSROOT)

$(BUILD)/sh: $(OBJ)
	$(CC) -o $@ $^

$(BUILD)/%.o: %.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MD -MP -c -o $@ $<

clean:
	rm -rf $(BUILD)

.PHONY: all install clean
