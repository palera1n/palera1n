NAME = The Beginning of the End

SRC = $(shell pwd)
DEP = $(SRC)/dep_root
STRIP = strip
CC ?= cc
CFLAGS += -I$(DEP)/include -I$(SRC)/include -I$(SRC)
CFLAGS += -Wall -Wextra -DPALERAIN_VERSION=\"2.0.0\" -Wall -Wextra -Wno-unused-parameter
CFLAGS += -Wno-unused-variable -I$(SRC)/src/palera1n -std=c99 -pedantic-errors -D_C99_SOURCE -D_POSIX_C_SOURCE=200112L -D_DARWIN_C_SOURCE
LIBS += $(DEP)/lib/libimobiledevice-1.0.a $(DEP)/lib/libirecovery-1.0.a $(DEP)/lib/libusbmuxd-2.0.a
LIBS += $(DEP)/lib/libimobiledevice-glue-1.0.a $(DEP)/lib/libplist-2.0.a -pthread
ifeq ($(TARGET_OS),)
TARGET_OS = $(shell uname -s)
endif
ifeq ($(TARGET_OS),Darwin)
CFLAGS += -Wno-nullability-extension
ifeq (,$(findstring version-min=, $(CFLAGS)))
CFLAGS += -mmacosx-version-min=10.8
endif
LDFLAGS += -Wl,-dead_strip
LIBS += -framework CoreFoundation -framework IOKit
else
CFLAGS += -fdata-sections -ffunction-sections
LDFLAGS += -static -no-pie -Wl,--gc-sections
endif
LIBS += $(DEP)/lib/libmbedtls.a $(DEP)/lib/libmbedcrypto.a $(DEP)/lib/libmbedx509.a $(DEP)/lib/libreadline.a

ifeq ($(DEV_BUILD),1)
LIBS += $(DEP)/lib/libnewt.a $(DEP)/lib/libpopt.a $(DEP)/lib/libslang.a
ifeq ($(TARGET_OS),Linux)
LIBS += $(DEP)/lib/libgpm.a
endif
CFLAGS += -O0 -g -DDEV_BUILD -fno-omit-frame-pointer
ifeq ($(ASAN),1)
BUILD_STYLE=ASAN
CFLAGS += -fsanitize=address,undefined -fsanitize-address-use-after-return=runtime
else ifeq ($(TSAN),1)
BUILD_STYLE=TSAN
CFLAGS += -fsanitize=thread,undefined
else
BUILD_STYLE = DEVELOPMENT
endif
else
CFLAGS += -Os -g
BUILD_STYLE = RELEASE
endif
LIBS += -lc

ifneq ($(BAKERAIN_DEVELOPE_R),)
CFLAGS += -DBAKERAIN_DEVELOPE_R="\"$(BAKERAIN_DEVELOPE_R)\""
endif

BUILD_DATE := $(shell LANG=C date)
BUILD_TAG := $(shell git describe --dirty --tags --abbrev=7)
BUILD_WHOAMI := $(shell whoami)

CFLAGS += -DBUILD_STYLE="\"$(BUILD_STYLE)\"" -DBUILD_DATE="\"$(BUILD_DATE)\"" -DBUILD_WHOAMI=\"$(BUILD_WHOAMI)\" -DBUILD_TAG=\"$(BUILD_TAG)\"

export SRC DEP CC CFLAGS LDFLAGS LIBS TARGET_OS DEV_BUILD BUILD_DATE BUILD_TAG BUILD_WHOAMI BUILD_STYLE

all: palera1n

palera1n: download-deps
	$(MAKE) -C src/palera1n

clean:
	$(MAKE) -C src/palera1n clean
	$(MAKE) -C docs clean

download-deps:
	$(MAKE) -C src/palera1n checkra1n-macos checkra1n-linux-arm64 checkra1n-linux-armel checkra1n-linux-x86 checkra1n-linux-x86_64 checkra1n-kpf-pongo ramdisk.dmg binpack.dmg

docs:
	$(MAKE) -C docs

distclean: clean
	rm -rf palera1n-* palera1n*.dSYM src/checkra1n-* src/checkra1n-kpf-pongo src/ramdisk.dmg src/binpack.dmg

.PHONY: all palera1n clean docs distclean

