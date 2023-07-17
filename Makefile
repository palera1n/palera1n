NAME = Dusk

SRC = $(shell pwd)
DEP = $(SRC)/dep_root
STRIP = strip
CC ?= cc
CFLAGS += -I$(DEP)/include -I$(SRC)/include -I$(SRC) -D_XOPEN_SOURCE=500
CFLAGS += -Wall -Wextra -Wno-unused-parameter -DPALERAIN_VERSION=\"2.0.0\"
CFLAGS += -Wno-unused-variable -I$(SRC)/src -std=c99 -pedantic-errors -D_C99_SOURCE -D_POSIX_C_SOURCE=200112L
ifeq ($(TARGET_OS),)
TARGET_OS = $(shell uname -s)
endif
ifeq ($(findstring MINGW64,$(TARGET_OS)),MINGW64)
TARGET_OS = Windows
CFLAGS += -DWIN
LIBS = -limobiledevice-1.0 -lirecovery-1.0 -lusbmuxd-2.0 -limobiledevice-glue-1.0 -lplist-2.0 -lssl -lcrypto -lusb-1.0 -lreadline -pthread -lm
LIBS += -lws2_32 -liphlpapi -lsetupapi -lwinusb
else
LIBS += $(DEP)/lib/libimobiledevice-1.0.a $(DEP)/lib/libirecovery-1.0.a $(DEP)/lib/libusbmuxd-2.0.a
LIBS += $(DEP)/lib/libimobiledevice-glue-1.0.a $(DEP)/lib/libplist-2.0.a -pthread -lm
LIBS += $(DEP)/lib/libmbedtls.a $(DEP)/lib/libmbedcrypto.a $(DEP)/lib/libmbedx509.a $(DEP)/lib/libreadline.a
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
LDFLAGS += -no-pie -Wl,--gc-sections
ifneq ($(TARGET_OS),Windows)
LDFLAGS += -static
endif
endif

ifeq ($(TUI),1)
LIBS += $(DEP)/lib/libnewt.a $(DEP)/lib/libpopt.a $(DEP)/lib/libslang.a
ifeq ($(TARGET_OS),Linux)
LIBS += $(DEP)/lib/libgpm.a
endif
endif
ifeq ($(DEV_BUILD),1)
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
#LIBS += -lc

ifneq ($(BAKERAIN_DEVELOPE_R),)
CFLAGS += -DBAKERAIN_DEVELOPE_R="\"$(BAKERAIN_DEVELOPE_R)\""
endif

BUILD_DATE := $(shell LANG=C date)
BUILD_NUMBER := $(shell git rev-list --count HEAD)
BUILD_TAG := $(shell git describe --dirty --tags --abbrev=7)
BUILD_WHOAMI := $(shell whoami)
BUILD_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)
BUILD_COMMIT := $(shell git rev-parse HEAD)

CFLAGS += -DBUILD_STYLE="\"$(BUILD_STYLE)\"" -DBUILD_DATE="\"$(BUILD_DATE)\""
CFLAGS += -DBUILD_WHOAMI="\"$(BUILD_WHOAMI)\"" -DBUILD_TAG="\"$(BUILD_TAG)\""
CFLAGS += -DBUILD_NUMBER="\"$(BUILD_NUMBER)\"" -DBUILD_BRANCH="\"$(BUILD_BRANCH)\""
CFLAGS += -DBUILD_COMMIT="\"$(BUILD_COMMIT)\""

export SRC DEP CC CFLAGS LDFLAGS LIBS TARGET_OS DEV_BUILD BUILD_DATE BUILD_TAG BUILD_WHOAMI BUILD_STYLE BUILD_NUMBER BUILD_BRANCH

all: palera1n

xxd-payloads: download-deps
	mkdir -p include/payloads
	for file in payloads/*; do \
		xxd -i $$file > include/$$file.h; \
	done

palera1n: download-deps xxd-payloads
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean
	$(MAKE) -C docs clean

download-deps: Pongo.bin
	$(MAKE) -C src checkra1n-kpf-pongo ramdisk.dmg binpack.dmg

docs:
	$(MAKE) -C docs

Pongo.bin:
	rm -f payloads/Pongo.bin
	curl -Lo payloads/Pongo.bin https://cdn.nickchan.lol/palera1n/artifacts/kpf/Pongo.bin

distclean: clean
	rm -rf palera1n-* palera1n*.dSYM src/checkra1n-* src/checkra1n-kpf-pongo src/ramdisk.dmg src/binpack.dmg payloads/Pongo.bin include/payloads

.PHONY: all xxd-payloads palera1n clean docs distclean

