.PHONY: payloads palera1n palera1n_xcode palera1n_mingw clean

WITH_GUI ?= 0
WITH_TUI ?= 0
WITH_RAMDISK ?= 1
WITH_BINPACK ?= 1
WITH_STATIC ?= 0
WITH_CIDERRAIN ?= 0
BUILD_TYPE ?= Debug
CMAKE_SYSROOT ?=
CROSS_HOST_TRIPLE ?=
JOBS ?=

BUILD_ARGS :=
ifneq ($(strip $(JOBS)),)
BUILD_ARGS += --parallel $(JOBS)
endif

payloads:
	mkdir -p src/gen/embedded

	xz --format=lzma -vfc6ekT 0 embedded/ramdisk.dmg > embedded/ramdisk-compressed.dmg.lzma
	xz --format=lzma -vfc6ekT 0 embedded/checkra1n-kpf-pongo > embedded/checkra1n-kpf-pongo-compressed.lzma

	@for file in embedded/*; do \
		name=$$(basename "$$file"); \
		name=$${name%.*}; \
		echo " XXD    $$file"; \
		echo "#pragma once" > "src/gen/embedded/$$name.h"; \
		xxd -i "$$file" \
		| sed 's/unsigned char/static const unsigned char/g' \
		| sed 's/unsigned int/static const size_t/g' \
		>> "src/gen/embedded/$$name.h"; \
	done

palera1n: payloads
	@cmake -S . -B build \
		-DCMAKE_SYSROOT="$(CMAKE_SYSROOT)" \
		-DCROSS_HOST_TRIPLE="$(CROSS_HOST_TRIPLE)" \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DWITH_GUI=$(WITH_GUI) \
		-DWITH_TUI=$(WITH_TUI) \
		-DWITH_RAMDISK=$(WITH_RAMDISK) \
		-DWITH_BINPACK=$(WITH_BINPACK) \
		-DWITH_CIDERRAIN=$(WITH_CIDERRAIN) \
		-DWITH_STATIC=$(WITH_STATIC) && \
	cmake --build build $(BUILD_ARGS)

palera1n_xcode: payloads
	@cmake -S . -B build \
		-G Xcode \
		-DCMAKE_SYSROOT="$(CMAKE_SYSROOT)" \
		-DCROSS_HOST_TRIPLE="$(CROSS_HOST_TRIPLE)" \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DWITH_GUI=$(WITH_GUI) \
		-DWITH_TUI=$(WITH_TUI) \
		-DWITH_RAMDISK=$(WITH_RAMDISK) \
		-DWITH_BINPACK=$(WITH_BINPACK) \
		-DWITH_CIDERRAIN=$(WITH_CIDERRAIN) \
		-DWITH_STATIC=$(WITH_STATIC)

palera1n_mingw: payloads
	@cmake -S . -B build \
		-G "MinGW Makefiles" \
		-DCMAKE_SYSROOT="$(CMAKE_SYSROOT)" \
		-DCROSS_HOST_TRIPLE="$(CROSS_HOST_TRIPLE)" \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DWITH_GUI=$(WITH_GUI) \
		-DWITH_TUI=$(WITH_TUI) \
		-DWITH_RAMDISK=$(WITH_RAMDISK) \
		-DWITH_BINPACK=$(WITH_BINPACK) \
		-DWITH_CIDERRAIN=$(WITH_CIDERRAIN) \
		-DWITH_STATIC=$(WITH_STATIC) && \
	cmake --build build $(BUILD_ARGS)

clean:
	@rm -rf build src/gen
