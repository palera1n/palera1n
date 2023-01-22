SRC = $(shell pwd)
DEP = $(SRC)/dep_root
STRIP = strip
CC ?= cc
CFLAGS += -I$(DEP)/include -I$(SRC)/include -I$(SRC) -Wall -Wextra -DPALERAIN_VERSION=\"2.0.0\" -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable
LIBS += $(DEP)/lib/libimobiledevice-1.0.a $(DEP)/lib/libirecovery-1.0.a $(DEP)/lib/libusbmuxd-2.0.a $(DEP)/lib/libimobiledevice-glue-1.0.a $(DEP)/lib/libplist-2.0.a -pthread
ifeq ($(TARGET_OS),)
TARGET_OS = $(shell uname -s)
endif
ifeq ($(TARGET_OS),Darwin)
CFLAGS += -mmacosx-version-min=10.8
LDFLAGS += -Wl,-dead_strip
LIBS += $(DEP)/lib/libcrypto.35.tbd $(DEP)/lib/libssl.35.tbd $(DEP)/lib/libusb-1.0.a
LIBS += -framework CoreFoundation -framework SystemConfiguration -framework IOKit -framework Security
else
CFLAGS += -fdata-sections -ffunction-sections
LDFLAGS += -static -no-pie -Wl,--gc-sections
LIBS += $(DEP)/lib/libssl.a $(DEP)/lib/libcrypto.a $(DEP)/lib/libreadline.a -latomic
endif
LIBS += $(DEP)/lib/libusb-1.0.a
LDFLAGS += -flto

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

macos-dist:
	$(MAKE) clean
	$(MAKE) all CFLAGS='-arch arm64 $(CFLAGS)'
	cp src/palera1n palera1n-arm64
	dsymutil src/palera1n
	cp -a src/palera1n.dSYM palera1n-arm64.dSYM
	$(MAKE) clean
	$(MAKE) all CFLAGS='-arch x86_64 $(CFLAGS)'
	cp src/palera1n palera1n-x86_64
	dsymutil src/palera1n
	cp -a src/palera1n.dSYM palera1n-x86_64.dSYM
	$(MAKE) clean
	lipo -create -arch arm64 palera1n-arm64 -arch x86_64 palera1n-x86_64 -output palera1n-macos
	mkdir -p palera1n-macos.dSYM/Contents/Resources/DWARF
	cp palera1n-arm64.dSYM/Contents/Info.plist palera1n-macos.dSYM/Contents
	lipo -create -arch arm64 palera1n-arm64.dSYM/Contents/Resources/DWARF/palera1n -arch x86_64 palera1n-x86_64.dSYM/Contents/Resources/DWARF/palera1n -output palera1n-macos.dSYM/Contents/Resources/DWARF/palera1n-macos
	$(STRIP) palera1n-macos
	codesign -s - --force palera1n-macos

download-checkra1n:
	$(MAKE) -C src checkra1n-macos checkra1n-linux-arm64 checkra1n-linux-armel checkra1n-linux-x86 checkra1n-linux-x86_64

distclean: clean
	rm -rf palera1n-* palera1n*.dSYM src/checkra1n-*

.PHONY: all palera1n clean

