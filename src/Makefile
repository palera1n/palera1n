all: palera1n

OBJECTS  = main.o dfuhelper.o devhelper.o lockdown_helper.o
OBJECTS += optparse.o override_file.o log.o lock_vars.o credits.o
OBJECTS += fake_embedded.o checkm8.o pongo_helper.o
OBJECTS += boyermoore_memmem.o

ifeq (,$(findstring macosx, $(CFLAGS)))
ifneq (,$(findstring version-min=, $(CFLAGS)))
OBJECTS += xxd-libcheckra1nhelper.dylib.o
endif
endif

ifeq ($(FORCE_HELPER),1)
OBJECTS += xxd-libcheckra1nhelper.dylib.o
CFLAGS += -DFORCE_HELPER
BUILD_OPTIONS += FORCE_HELPER
endif

ifneq ($(NO_RAMDISK),1)
OBJECTS += xxd-ramdisk.dmg.o
else
CFLAGS += -DNO_RAMDISK
BUILD_OPTIONS += NO_RAMDISK
endif
ifneq ($(NO_KPF),1)
OBJECTS += xxd-checkra1n-kpf-pongo.o
else
CFLAGS += -DNO_KPF
BUILD_OPTIONS += NO_KPF
endif
ifneq ($(NO_OVERLAY),1)
OBJECTS += xxd-binpack.dmg.o
else
CFLAGS += -DNO_OVERLAY
BUILD_OPTIONS += NO_OVERLAY
endif

ifeq ($(TUI),1)
OBJECTS += tui_screen_enter_recovery.o tui_screen_enter_dfu.o tui_main.o
OBJECTS += tui_screen_jailbreak.o tui_screen_main.o tui_screen_options.o
BUILD_OPTIONS += TUI
CFLAGS += -DTUI
endif

LIPO = lipo
LDID = ldid

ifeq ($(TARGET_OS),Darwin)
ifneq ($(USE_LIBUSB),1)
OBJECTS += usb_iokit.o
else
LIBS += -framework SystemConfiguration -framework Security
BUILD_OPTIONS += USE_LIBUSB
endif
# add lto and debug symbols for lto
CFLAGS += -flto=full -g
LDFLAGS += -Wl,-object_path_lto,lto.o
else
USE_LIBUSB = 1
endif

ifeq ($(USE_LIBUSB),1)
OBJECTS += usb_libusb.o
CFLAGS += -DHAVE_LIBUSB -DUSE_LIBUSB=1
LIBS += $(DEP)/lib/libusb-1.0.a
endif

ifeq ($(ROOTFUL),1)
CFLAGS += -DROOTFUL=1
BUILD_OPTIONS += ROOTFUL
endif

ifeq ($(BUILD_OPTIONS),)
BUILD_OPTIONS = (none)
endif

LIBS += lz4/lz4.c lz4/lz4hc.c
CFLAGS += -DBUILD_OPTIONS=\""$(BUILD_OPTIONS)"\" -Wno-pointer-sign -Wno-gnu-pointer-arith -Wno-pedantic

xxd-%.c: %
	xxd -iC $< > $@

%.o: %.c $(wildcard ../include/*.h)
	$(CC) -c $(CFLAGS) $<

palera1n: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) $(LIBS) -o palera1n
ifeq (,$(findstring macosx, $(CFLAGS)))
ifneq (,$(findstring version-min=, $(CFLAGS)))
	$(LDID) -Susb.xml palera1n
endif
endif

libcheckra1nhelper.dylib: chkstk.S
	$(CC) $(CFLAGS) $(LDFLAGS) -shared chkstk.S -o libcheckra1nhelper.dylib

checkra1n-kpf-pongo:
	curl -LO https://cdn.nickchan.lol/palera1n/artifacts/kpf/checkra1n-kpf-pongo

ramdisk.dmg:
	curl -LO https://cdn.nickchan.lol/palera1n/c-rewrite/deps/ramdisk.dmg

binpack.dmg:
	curl -LO https://cdn.nickchan.lol/palera1n/c-rewrite/deps/binpack.dmg

clean:
	rm -rf *.o palera1n checkra1n checkra1n.c palera1n.dSYM libcheckra1nhelper.dylib

.PHONY: all clean
