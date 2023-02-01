SRC = $(shell pwd)
DEP = $(SRC)/dep_root
STRIP = strip
CC ?= cc
CFLAGS += -I$(DEP)/include -I$(DEP)/include/ncursestw -I$(SRC)/include -I$(SRC)
CFLAGS += -Wall -Wextra -DPALERAIN_VERSION=\"2.0.0\" -Wall -Wextra -Wno-unused-parameter
CFLAGS += -Wno-unused-variable -I$(SRC)/src
LIBS += $(DEP)/lib/libimobiledevice-1.0.a $(DEP)/lib/libirecovery-1.0.a $(DEP)/lib/libusbmuxd-2.0.a
LIBS += $(DEP)/lib/libimobiledevice-glue-1.0.a $(DEP)/lib/libplist-2.0.a -pthread
ifeq ($(TARGET_OS),)
TARGET_OS = $(shell uname -s)
endif
ifeq ($(TARGET_OS),Darwin)
ifeq (,$(findstring version-min=, $(CFLAGS)))
CFLAGS += -mmacosx-version-min=10.8
endif
LDFLAGS += -Wl,-dead_strip
LIBS += -framework CoreFoundation -framework SystemConfiguration -framework IOKit -framework Security
else
CFLAGS += -fdata-sections -ffunction-sections
LDFLAGS += -static -no-pie -Wl,--gc-sections
endif
LIBS += $(DEP)/lib/libmbedtls.a $(DEP)/lib/libmbedcrypto.a $(DEP)/lib/libmbedx509.a $(DEP)/lib/libreadline.a
LIBS += $(DEP)/lib/libusb-1.0.a

ifeq ($(DEV_BUILD),1)
LIBS += $(DEP)/lib/libmenutw.a $(DEP)/lib/libncursestw.a
CFLAGS += -O0 -g -DDEV_BUILD -fno-omit-frame-pointer
ifeq ($(ASAN),1)
CFLAGS += -DBUILD_STYLE=\"ASAN\" -fsanitize=address,undefined -fsanitize-address-use-after-return=runtime
else
CFLAGS += -DBUILD_STYLE=\"DEVELOPMENT\"
endif
else
CFLAGS += -Os -g -DBUILD_STYLE=\"RELEASE\"
endif
CFLAGS += -DBUILD_DATE="\"$(shell LANG=C date)\"" -DBUILD_WHOAMI=\"$(shell whoami)\" -DBUILD_TAG=\"$(shell git describe --dirty --tags --abbrev=7)\"

export SRC DEP CC CFLAGS LDFLAGS LIBS TARGET_OS DEV_BUILD

all: palera1n

palera1n: download-deps
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

download-deps:
	$(MAKE) -C src checkra1n-macos checkra1n-linux-arm64 checkra1n-linux-armel checkra1n-linux-x86 checkra1n-linux-x86_64 checkra1n-kpf-pongo ramdisk.dmg binpack.dmg

distclean: clean
	rm -rf palera1n-* palera1n*.dSYM src/checkra1n-* src/checkra1n-kpf-pongo src/ramdisk.dmg src/binpack.dmg

.PHONY: all palera1n clean

