.PHONY: payloads palera1n palera1n_xcode clean

WITH_GUI ?= 0
WITH_TUI ?= 0
WITH_ARTIFACTS ?= 1
WITH_STATIC ?= 0
BUILD_TYPE ?= Debug

payloads:
	mkdir -p src/gen/images
	mkdir -p src/gen/payloads

	@for file in payloads/*; do \
		name=$$(basename "$$file"); \
		name=$${name%.*}; \
		echo " XXD    $$file"; \
		xxd -i "$$file" > "src/gen/payloads/$$name.h"; \
	done

	@for file in images/*; do \
		name=$$(basename "$$file"); \
		name=$${name%.*}; \
		echo " XXD    $$file"; \
		xxd -i "$$file" > "src/gen/images/$$name.h"; \
	done

	xxd -i -n "DFUHelperDeviceInfo" resources/DFUHelperDeviceInfo.json > src/gen/DFUHelperDeviceInfo.h

palera1n: payloads
	@cmake -S . -B build \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DWITH_GUI=$(WITH_GUI) \
		-DWITH_TUI=$(WITH_TUI) \
		-DWITH_ARTIFACTS=$(WITH_ARTIFACTS) \
		-DWITH_STATIC=$(WITH_STATIC) && \
	cmake --build build

palera1n_xcode: payloads
	@cmake -S . -B build \
		-G Xcode \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DWITH_GUI=$(WITH_GUI) \
		-DWITH_TUI=$(WITH_TUI) \
		-DWITH_ARTIFACTS=$(WITH_ARTIFACTS) \
		-DWITH_STATIC=$(WITH_STATIC)

palera1n_mingw: payloads
	@cmake -S . -B build \
		-G "MinGW Makefiles" \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DWITH_GUI=$(WITH_GUI) \
		-DWITH_TUI=$(WITH_TUI) \
		-DWITH_ARTIFACTS=$(WITH_ARTIFACTS) \
		-DWITH_STATIC=$(WITH_STATIC) && \
	cmake --build build

clean:
	@rm -rf build
