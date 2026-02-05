.PHONY: payloads palera1n palera1n_xcode clean

WITH_GUI ?= 0
WITH_STATIC ?= 0
BUILD_TYPE ?= Debug

payloads:
	@mkdir -p src/exploit/payloads
	@for file in payloads/*; do \
		echo " XXD    $$file"; \
		xxd -i $$file > src/exploit/$$file.h; \
	done

palera1n: payloads
	@cmake -S . -B build \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DWITH_GUI=$(WITH_GUI) \
		-DWITH_STATIC=$(WITH_STATIC) && \
	cmake --build build -- -j$(sysctl -n hw.ncpu)

palera1n_xcode: payloads
	@cmake -S . -B build \
		-G Xcode \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DWITH_GUI=$(WITH_GUI) \
		-DWITH_STATIC=$(WITH_STATIC)

clean:
	@rm -rf build
