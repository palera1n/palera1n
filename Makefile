.PHONY: all clean

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------

WITH_GUI          ?= 0
WITH_TUI          ?= 0
WITH_RAMDISK      ?= 1
WITH_BINPACK      ?= 1
WITH_STATIC       ?= 0
WITH_CIDERRAIN    ?= 0
BUILD_TYPE        ?= Debug

CMAKE_SYSROOT     ?=
CROSS_HOST_TRIPLE ?=
JOBS              ?= 1

PLATFORM ?= host

UNAME_S := $(shell uname -s)

ifeq ($(PLATFORM),iphoneos)
	ifneq ($(UNAME_S),Darwin)
		$(error iOS targets are only supported on macOS)
	endif

	CMAKE_GENERATOR := -G Xcode
	APPLE_SYSROOT := $(shell xcrun --sdk iphoneos --show-sdk-path)

	CMAKE_PLATFORM_ARGS := \
		-DCMAKE_SYSTEM_NAME=iOS \
		-DCMAKE_OSX_SYSROOT="$(APPLE_SYSROOT)" \
		-DCMAKE_OSX_ARCHITECTURES=arm64

else ifeq ($(UNAME_S),Windows_NT)
	CMAKE_GENERATOR := -G "MinGW Makefiles"
else
	CMAKE_GENERATOR :=
endif

CMAKE_ARGS := \
	$(CMAKE_GENERATOR) \
	-DCMAKE_SYSROOT="$(CMAKE_SYSROOT)" \
	-DCROSS_HOST_TRIPLE="$(CROSS_HOST_TRIPLE)" \
	-DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" \
	-DWITH_GUI=$(WITH_GUI) \
	-DWITH_TUI=$(WITH_TUI) \
	-DWITH_RAMDISK=$(WITH_RAMDISK) \
	-DWITH_BINPACK=$(WITH_BINPACK) \
	-DWITH_STATIC=$(WITH_STATIC) \
	-DWITH_CIDERRAIN=$(WITH_CIDERRAIN) \
	$(CMAKE_PLATFORM_ARGS)

ifneq ($(strip $(JOBS)),)
	BUILD_ARGS := --parallel $(JOBS)
endif

all: palera1n

palera1n:
	@cmake -S . -B build $(CMAKE_ARGS)
	@cmake --build build $(BUILD_ARGS)
ifeq ($(PLATFORM),iphoneos)
	@codesign --force --sign - --entitlements resources/entitlements.xml build/Debug-iphoneos/palera1n.app/palera1n
endif

clean:
	@rm -rf build src/gen apple-include-*
	@if [ -d 1stparty/libciderra1n ]; then \
		$(MAKE) -C 1stparty/libciderra1n clean; \
	fi
	@$(MAKE) -C 1stparty/libopenra1n clean
