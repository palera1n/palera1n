SRC = $(shell pwd)
DEP = $(SRC)/dep_root
STRIP = strip
CC ?= cc
CFLAGS += -I$(DEP)/include -I$(SRC)/include -I$(SRC) -Wall -Wextra -DPALERAIN_VERSION=\"2.0.0\" -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wformat
LIBS += $(DEP)/lib/libimobiledevice-1.0.a $(DEP)/lib/libirecovery-1.0.a $(DEP)/lib/libusbmuxd-2.0.a $(DEP)/lib/libimobiledevice-glue-1.0.a $(DEP)/lib/libplist-2.0.a -pthread
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
CFLAGS += -O0 -g
else
CFLAGS += -Os -g
endif

export SRC DEP CC CFLAGS LDFLAGS LIBS TARGET_OS

all: palera1n

palera1n: download-checkra1n
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

download-checkra1n:
	$(MAKE) -C src checkra1n-macos checkra1n-linux-arm64 checkra1n-linux-armel checkra1n-linux-x86 checkra1n-linux-x86_64

distclean: clean
	rm -rf palera1n-* palera1n*.dSYM src/checkra1n-*

.PHONY: all palera1n clean

