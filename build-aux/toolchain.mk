# Copyright (c) 2016, 2019 Dennis Wölfing
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

# Don't use the default values from make.
ifneq ($(filter default undefined, $(origin AR)),)
  AR = $(ARCH)-cobalt-ar
endif

ifneq ($(filter default undefined, $(origin CC)),)
  CC = $(ARCH)-cobalt-gcc
endif

ifneq ($(filter default undefined, $(origin CXX)),)
  CXX = $(ARCH)-cobalt-g++
endif

CPP = $(CC) -E
MKRESCUE ?= grub-mkrescue
