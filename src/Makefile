all: palera1n

OBJECTS  = main.o dfuhelper.o devhelper.o lockdown_helper.o
OBJECTS += optparse.o override_file.o log.o lock_vars.o credits.o
OBJECTS += fake_embedded.o exec_checkra1n.o pongo_helper.o
OBJECTS += boyermoore_memmem.o

ifeq (,$(findstring macosx, $(CFLAGS)))
ifneq (,$(findstring version-min=, $(CFLAGS)))
OBJECTS += xxd-libcheckra1nhelper.dylib.o
endif
endif

ifeq ($(FORCE_HELPER),1)
OBJECTS += xxd-libcheckra1nhelper.dylib.o
CFLAGS += -DFORCE_HELPER
endif

ifneq ($(NO_RAMDISK),1)
OBJECTS += xxd-ramdisk.dmg.o
else
CFLAGS += -DNO_RAMDISK
endif
ifneq ($(NO_KPF),1)
OBJECTS += xxd-checkra1n-kpf-pongo.o
else
CFLAGS += -DNO_KPF
endif
ifneq ($(NO_BINPACK),1)
OBJECTS += xxd-binpack.dmg.o
else
CFLAGS += -DNO_BINPACK
endif
ifneq ($(NO_CHECKRAIN),1)
OBJECTS += xxd-checkra1n.o
else
CFLAGS += -DNO_CHECKRAIN
endif

ifeq ($(DEV_BUILD),1)
OBJECTS += tui_screen_enter_recovery.o tui_screen_enter_dfu.o tui_main.o
OBJECTS += tui_screen_jailbreak.o tui_screen_main.o tui_screen_options.o
endif

CHECKRA1N_VERSION ?= 0.1337.1
LIPO = lipo
LDID = ldid

ifeq ($(TARGET_OS),Darwin)
ifneq ($(USE_LIBUSB),1)
OBJECTS += usb_iokit.o
else
LIBS += -framework SystemConfiguration -framework Security
endif
else
USE_LIBUSB = 1
endif

ifeq ($(USE_LIBUSB),1)
OBJECTS += usb_libusb.o
CFLAGS += -DUSE_LIBUSB=1
LIBS += $(DEP)/lib/libusb-1.0.a
endif

checkra1n-%:
	curl -LO https://assets.checkra.in/downloads/preview/$(CHECKRA1N_VERSION)/$@
	chmod 755 $@

checkra1n: checkra1n-macos usb.xml
ifeq ($(TARGET_OS),Darwin)
ifneq (,$(findstring arm64, $(CFLAGS)))
	$(LIPO) -thin arm64 -output checkra1n checkra1n-macos
else ifneq (,$(findstring x86_64, $(CFLAGS)))
	$(LIPO) -thin x86_64 -output checkra1n checkra1n-macos
else ifeq ($(shell arch),i386)
	$(LIPO) -thin x86_64 -output checkra1n checkra1n-macos
else
	$(LIPO) -thin $(shell arch) -output checkra1n checkra1n-macos
endif
ifeq (,$(findstring macosx, $(CFLAGS)))
ifneq (,$(findstring version-min=, $(CFLAGS)))
	vtool -remove-build-version 1 -replace -output checkra1n checkra1n
	vtool -set-build-version $(FAKE_PLATFORM) $(FAKE_MINOS) $(FAKE_SDK) -replace -output checkra1n checkra1n
	install_name_tool -change /System/Library/Frameworks/IOKit.framework/Versions/A/IOKit /System/Library/Frameworks/IOKit.framework/IOKit checkra1n
	install_name_tool -change /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation /System/Library/Frameworks/CoreFoundation.framework/CoreFoundation checkra1n
	$(LDID) -Susb.xml checkra1n
endif
endif
else
	cp checkra1n-$(CHECKRA1N_NAME) checkra1n
endif

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
