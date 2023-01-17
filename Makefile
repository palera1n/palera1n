SRC = $(shell pwd)
DEP = $(SRC)/dep_root
CC = cc
CFLAGS += -mmacosx-version-min=10.8 -I$(DEP)/include -I/opt/procursus/include -I$(SRC)/include -I$(SRC) -O0 -g -Wall -Wextra
LIBS = $(DEP)/lib/libimobiledevice-1.0.a $(DEP)/lib/libirecovery-1.0.a $(DEP)/lib/libusbmuxd-2.0.a $(DEP)/lib/libplist-2.0.a $(DEP)/lib/libimobiledevice-glue-1.0.a -pthread
LIBS += $(DEP)/lib/libcrypto.35.tbd $(DEP)/lib/libssl.35.tbd $(DEP)/lib/libusb-1.0.a
LIBS += -framework CoreFoundation -framework SystemConfiguration -framework IOKit -framework Security
LDFLAGS = $(LIBS)

export SRC DEP CC CFLAGS LDFLAGS LIBS

all: palera1n

palera1n:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

dist:
	$(MAKE) clean
	$(MAKE) all CFLAGS='-arch arm64 $(CFLAGS)'
	cp src/palera1n palera1n-arm64
	$(MAKE) clean
	$(MAKE) all CFLAGS='-arch x86_64 $(CFLAGS)'
	cp src/palera1n palera1n-x86_64
	$(MAKE) clean
	lipo -create -arch arm64 palera1n-arm64 -arch x86_64 palera1n-x86_64 -output palera1n-macos

distclean: clean
	rm -f palera1n-*

.PHONY: all palera1n clean
