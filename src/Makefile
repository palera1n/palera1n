all: palera1n

OBJECTS  = main.o dfuhelper.o devhelper.o lockdown_helper.o exec_checkra1n.o
OBJECTS += pongo_usb.o optparse.o override_file.o pongo_helper.o log.o
OBJECTS += xxd-checkra1n.o xxd-checkra1n-kpf-pongo.o xxd-ramdisk.dmg.o
OBJECTS += xxd-binpack.dmg.o tui_main.o tui_screen_options.o
OBJECTS += tui_screen_enter_recovery.o tui_screen_enter_dfu.o
OBJECTS += tui_screen_jailbreak.o tui_screen_main.o

CHECKRA1N_VERSION ?= 0.1337.1
LIPO = lipo
LDID = ldid

checkra1n-%:
	curl -LO https://assets.checkra.in/downloads/preview/$(CHECKRA1N_VERSION)/$@
	chmod 755 $@

checkra1n: checkra1n-macos usb.xml
ifeq ($(TARGET_OS),Darwin)
ifneq (,$(findstring arm64, $(CFLAGS)))
	$(LIPO) -thin arm64 -output checkra1n checkra1n-macos
else ifneq (,$(findstring x86_64, $(CFLAGS)))
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

%.o: %.c
	$(CC) -c $(CFLAGS) $<

palera1n: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) $(LIBS) -o palera1n
ifeq (,$(findstring macosx, $(CFLAGS)))
ifneq (,$(findstring version-min=, $(CFLAGS)))
	$(LDID) -Susb.xml palera1n
endif
endif

clean:
	rm -rf *.o palera1n checkra1n checkra1n.c palera1n.dSYM

.PHONY: all clean
