SRC = $(shell pwd)
DEP = $(SRC)/dep_root
CC = cc
CFLAGS = -mmacosx-version-min=12.0 -I$(DEP)/include -I/opt/procursus/include -I$(SRC)/include -g -O0 -Wall -Wextra
LIBS = $(DEP)/lib/libimobiledevice-1.0.a $(DEP)/lib/libirecovery-1.0.a $(DEP)/lib/libusbmuxd-2.0.a $(DEP)/lib/libplist-2.0.a $(DEP)/lib/libimobiledevice-glue-1.0.a -pthread
LIBS += $(DEP)/lib/libcrypto.35.tbd $(DEP)/lib/libssl.35.tbd 
LIBS += -framework CoreFoundation -framework SystemConfiguration -framework IOKit
LDFLAGS = $(LIBS)

export SRC DEP CC CFLAGS LDFLAGS

all: palera1n

palera1n:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

.PHONY: all palera1n clean
