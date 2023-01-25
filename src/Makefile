all: palera1n

OBJECTS = main.o dfuhelper.o devhelper.o lockdown_helper.o exec_checkra1n.o pongo_usb.o checkra1n.o log.o
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

checkra1n.c: checkra1n checkra1n-kpf-pongo ramdisk.dmg binpack.dmg
	xxd -iC checkra1n > checkra1n.c
	xxd -iC checkra1n-kpf-pongo >> checkra1n.c
	xxd -iC ramdisk.dmg >> checkra1n.c
	xxd -iC binpack.dmg >> checkra1n.c

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
