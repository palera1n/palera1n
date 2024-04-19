all: palera1n

OBJDIR = obj
GENDIR = gen
RESOURCES_DIR = resources

C_SRC = $(wildcard *.c)
OBJS = $(patsubst %,$(OBJDIR)/%,$(C_SRC:.c=.c.o))

ifeq (,$(findstring macosx, $(CFLAGS)))
ifneq (,$(findstring version-min=, $(CFLAGS)))
ifneq ($(NO_EMBED_HELPER),1)
C_SRC += $(GENDIR)/gen-libcheckra1nhelper.dylib.c
endif
endif
endif

ifeq ($(NO_EMBED_HELPER),1)
CFLAGS += -DNO_EMBED_HELPER
endif

ifeq ($(DFUHELPER_AUTO_ONLY),1)
CFLAGS += -DDFUHELPER_AUTO_ONLY
endif

ifeq ($(FORCE_HELPER),1)
ifneq ($(NO_EMBED_HELPER),1)
C_SRC += $(GENDIR)/gen-libcheckra1nhelper.dylib.c
endif
CFLAGS += -DFORCE_HELPER
BUILD_OPTIONS += FORCE_HELPER
endif

ifneq ($(NO_RAMDISK),1)
C_SRC += $(GENDIR)/gen-ramdisk.dmg.lzma.c
else
CFLAGS += -DNO_RAMDISK -DRAMDISK_COMPRESSED_SIZE=\"0\"
BUILD_OPTIONS += NO_RAMDISK
endif
ifneq ($(NO_KPF),1)
C_SRC += $(GENDIR)/gen-checkra1n-kpf-pongo.lzma.c
else
CFLAGS += -DNO_KPF
BUILD_OPTIONS += NO_KPF
endif
ifneq ($(NO_OVERLAY),1)
C_SRC += $(GENDIR)/gen-binpack.dmg.c
else
CFLAGS += -DNO_OVERLAY
BUILD_OPTIONS += NO_OVERLAY
endif
ifneq ($(NO_CHECKRAIN),1)
C_SRC += $(GENDIR)/gen-checkra1n.c
else
CFLAGS += -DNO_CHECKRAIN
BUILD_OPTIONS += NO_CHECKRAIN
endif

ifneq ($(NO_CUSTOM_PONGO),1)
C_SRC += $(GENDIR)/gen-Pongo.bin.c
else
CFLAGS += -DNO_CUSTOM_PONGO
BUILD_OPTIONS += NO_CUSTOM_PONGO
endif

CHECKRA1N_VERSION ?= 0.1337.2
LIPO = lipo
LDID = ldid

ifeq ($(TARGET_OS),Darwin)
ifneq ($(USE_LIBUSB),1)
else
LIBS += -framework SystemConfiguration -framework Security
BUILD_OPTIONS += USE_LIBUSB
endif
# add lto and debug symbols for lto
CFLAGS += -flto=full -g
LDFLAGS += -Wl,-object_path_lto,obj/lto.o
else
USE_LIBUSB = 1
endif

ifeq ($(USE_LIBUSB),1)
CFLAGS += -DUSE_LIBUSB=1
LIBS += $(DEP)/lib/libusb-1.0.a
endif

ifneq ($(ROOTFUL),0)
CFLAGS += -DROOTFUL=1
BUILD_OPTIONS += ROOTFUL
endif

ifeq ($(BUILD_OPTIONS),)
BUILD_OPTIONS = (none)
endif

CFLAGS += -DBUILD_OPTIONS=\""$(BUILD_OPTIONS)"\"

ifeq ($(TARGET_OS),Linux)
ifeq ($(UNAME),Linux)
ifeq ($(CHECKRA1N_NAME),)
ifeq ($(shell arch),x86_64)
CHECKRA1N_NAME := linux-x86_64
else ifeq ($(shell arch | grep -q 'i[3-7]86' && echo 1),1)
CHECKRA1N_NAME := linux-x86
else ifeq ($(shell arch | grep -v '\(arm\|aarch\)[a-z]\?\+64' | grep -q arm && echo 1),1)
CHECKRA1N_NAME := linux-armel
else ifeq  ($(shell arch | grep -q '\(arm\|aarch\)[a-z]\?\+64' && echo 1),1)
CHECKRA1N_NAME := linux-arm64
endif
endif
endif
endif

$(RESOURCES_DIR)/checkra1n-%:
	curl -Lfo $@ https://assets.checkra.in/downloads/preview/$(CHECKRA1N_VERSION)/$$(echo $@ | cut -d/ -f2)
	chmod 755 $@

$(RESOURCES_DIR)/checkra1n: $(RESOURCES_DIR)/checkra1n-macos usb.xml
ifeq ($(TARGET_OS),Darwin)
ifneq (,$(findstring arm64, $(CFLAGS)))
	$(LIPO) -thin arm64 -output $(RESOURCES_DIR)/checkra1n $(RESOURCES_DIR)/checkra1n-macos
else ifneq (,$(findstring x86_64, $(CFLAGS)))
	$(LIPO) -thin x86_64 -output $(RESOURCES_DIR)/checkra1n $(RESOURCES_DIR)/checkra1n-macos
else ifeq ($(shell arch),i386)
	$(LIPO) -thin x86_64 -output $(RESOURCES_DIR)/checkra1n $(RESOURCES_DIR)/checkra1n-macos
else
	$(LIPO) -thin $(shell arch) -output $(RESOURCES_DIR)/checkra1n $(RESOURCES_DIR)/checkra1n-macos
endif
ifeq (,$(findstring macosx, $(CFLAGS)))
ifneq (,$(findstring version-min=, $(CFLAGS)))
	vtool -remove-build-version 1 -replace -output $(RESOURCES_DIR)/checkra1n $(RESOURCES_DIR)/checkra1n
	vtool -set-build-version $(FAKE_PLATFORM) $(FAKE_MINOS) $(FAKE_SDK) -replace -output $(RESOURCES_DIR)/checkra1n $(RESOURCES_DIR)/checkra1n
	install_name_tool -change /System/Library/Frameworks/IOKit.framework/Versions/A/IOKit /System/Library/Frameworks/IOKit.framework/IOKit $(RESOURCES_DIR)/checkra1n
	install_name_tool -change /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation /System/Library/Frameworks/CoreFoundation.framework/CoreFoundation $(RESOURCES_DIR)/checkra1n
	$(LDID) -Susb.xml $(RESOURCES_DIR)/checkra1n
endif
endif
else
	cp $(RESOURCES_DIR)/checkra1n-$(CHECKRA1N_NAME) $(RESOURCES_DIR)/checkra1n
endif

$(GENDIR)/gen-%.c: $(RESOURCES_DIR)/%
	echo '#include <stdint.h>' > $(RESOURCES_DIR)/../$@
	cd $(RESOURCES_DIR) && xxd -iC $$(echo $< | cut -d/ -f2) | sed -e 's/unsigned int/uint32_t/' -e 's/unsigned char/uint8_t/' >> ../$@


$(OBJDIR)/pongo_helper.c.o: pongo_helper.c $(wildcard ../include/*.h) $(RESOURCES_DIR)/ramdisk.dmg $(RESOURCES_DIR)/checkra1n-kpf-pongo
	$(CC) -c -o $@ $(CFLAGS) -DRAMDISK_UNCOMPRESSED_SIZE="\"$$(wc -c < $(RESOURCES_DIR)/ramdisk.dmg | tr -d ' ')\"" -DKPF_UNCOMPRESSED_SIZE="\"$$(wc -c < $(RESOURCES_DIR)/checkra1n-kpf-pongo | tr -d ' ')\"" $<

$(OBJDIR)/%.c.o: $(GENDIR)/gen-%.c $(wildcard ../include/*.h)
	$(CC) $(CFLAGS) -c -o $<

$(OBJDIR)/%.c.o: %.c $(wildcard ../include/*.h)
	$(CC) $(CFLAGS) -c -o $@ $<

palera1n: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o palera1n
ifeq (,$(findstring macosx, $(CFLAGS)))
ifneq (,$(findstring version-min=, $(CFLAGS)))
	$(LDID) -Susb.xml palera1n
endif
endif

$(RESOURCES_DIR)/libcheckra1nhelper.dylib: chkstk.S
	$(CC) $(CFLAGS) $(LDFLAGS) -shared chkstk.S -o $@

$(RESOURCES_DIR)/checkra1n-kpf-pongo:
	curl -Lfo $@ https://cdn.nickchan.lol/palera1n/artifacts/kpf/iOS15/checkra1n-kpf-pongo

$(RESOURCES_DIR)/ramdisk.dmg.lzma: $(RESOURCES_DIR)/ramdisk.dmg
	xz --format=lzma -vfc6ekT 0 $< > $@

$(RESOURCES_DIR)/checkra1n-kpf-pongo.lzma: $(RESOURCES_DIR)/checkra1n-kpf-pongo
	xz --format=lzma -vfc6ekT 0 $< > $@

$(RESOURCES_DIR)/ramdisk.dmg:
	curl -Lfo $@ https://cdn.nickchan.lol/palera1n/c-rewrite/deps/ramdisk.dmg

$(RESOURCES_DIR)/binpack.dmg:
	curl -Lfo $@ https://cdn.nickchan.lol/palera1n/c-rewrite/deps/binpack.dmg

$(RESOURCES_DIR)/Pongo.bin:
	curl -Lfo $@ https://cdn.nickchan.lol/palera1n/artifacts/kpf/iOS15/Pongo.bin

clean:
	rm -rf $(OBJDIR)/*.o $(GENDIR)/*.c $(GENDIR)/*.o palera1n palera1n.dSYM libcheckra1nhelper.dylib

distclean: clean
	rm -rf $(RESOURCES_DIR)/*

.PHONY: all clean
