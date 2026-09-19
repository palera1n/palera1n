# =========================
# MARK: NON-CMAKE DEPENDENCIES: IDEVICE
# =========================


set(IDevice_CARGO_ARGS
    --release
    --no-default-features
    --features "usbmuxd ring tcp"
)

set(IDevice_ENV)

if(APPLE)
    if(IOS)
        list(APPEND IDevice_ENV "IPHONEOS_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    else()
        list(APPEND IDevice_ENV "MACOSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()
endif()

if(CROSS_HOST_TRIPLE)
    if(CROSS_HOST_TRIPLE STREQUAL "i486-linux-musl")
        set(RUST_TARGET "i686-unknown-linux-musl")
    elseif(CROSS_HOST_TRIPLE STREQUAL "x86_64-linux-musl")
        set(RUST_TARGET "x86_64-unknown-linux-musl")
    elseif(CROSS_HOST_TRIPLE STREQUAL "armel-linux-musleabi")
        set(RUST_TARGET "arm-unknown-linux-musleabi")
    elseif(CROSS_HOST_TRIPLE STREQUAL "aarch64-linux-musl")
        set(RUST_TARGET "aarch64-unknown-linux-musl")
    endif()

    if(RUST_TARGET)
        string(TOUPPER "${RUST_TARGET}" RUST_TARGET_ENV)
        string(REPLACE "-" "_" RUST_TARGET_ENV "${RUST_TARGET_ENV}")

        list(APPEND IDevice_CARGO_ARGS --target ${RUST_TARGET})
        list(APPEND IDevice_ENV
            "CC=${CMAKE_C_COMPILER}"
            "AR=${CMAKE_AR}"
            "CARGO_TARGET_${RUST_TARGET_ENV}_LINKER=${CMAKE_C_COMPILER}"
        )
    endif()

elseif(MINGW)
    set(RUST_TARGET "x86_64-pc-windows-gnu")

    list(APPEND IDevice_CARGO_ARGS --target ${RUST_TARGET})

    list(APPEND IDevice_ENV
        "CC=${CMAKE_C_COMPILER}"
        "AR=${CMAKE_AR}"
        "CARGO_TARGET_X86_64_PC_WINDOWS_GNU_LINKER=${CMAKE_C_COMPILER}"
    )
endif()

# Only build idevice if libraries are missing
set(IDEVICE_INSTALL_DIR "${CMAKE_BINARY_DIR}/idevice_ep-prefix")
if(NOT (EXISTS "${IDEVICE_INSTALL_DIR}/lib/idevice/libidevice++.a"
    AND EXISTS "${IDEVICE_INSTALL_DIR}/lib/idevice/libidevice_ffi.a"))

    ExternalProject_Add(idevice_ep
        GIT_REPOSITORY https://github.com/jkcoxson/idevice.git
        GIT_TAG v0.1.65

        PATCH_COMMAND
            patch --forward --ignore-whitespace -p1 < "${CMAKE_SOURCE_DIR}/patches/idevice/0001-compile-usbmuxd-and-windows-fixes.patch" || true

        CONFIGURE_COMMAND ""

        BUILD_COMMAND
            ${CMAKE_COMMAND} -E env
                ${IDevice_ENV}
                cargo build
                ${IDevice_CARGO_ARGS}
                --manifest-path <SOURCE_DIR>/ffi/Cargo.toml

            COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR>/cpp
                ${CMAKE_COMMAND}
                    -B build
                    -DCMAKE_BUILD_TYPE=Release
                    -DIDEVICE_FFI_PATH=<SOURCE_DIR>/target/${RUST_TARGET}/release/${FFI_LIB_NAME}
                    "-DCMAKE_CXX_FLAGS=-Du_int64_t=uint64_t -Du_int32_t=uint32_t -Du_int16_t=uint16_t -Du_int8_t=uint8_t"
                    "-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}"

            COMMAND ${CMAKE_COMMAND} --build <SOURCE_DIR>/cpp/build --config Release

        INSTALL_COMMAND
            ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/lib/idevice
            COMMAND ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/include

            COMMAND ${CMAKE_COMMAND} -E copy
                <SOURCE_DIR>/target/${RUST_TARGET}/release/libidevice_ffi.a
                <INSTALL_DIR>/lib/idevice/libidevice_ffi.a

            COMMAND ${CMAKE_COMMAND} -E copy
                <SOURCE_DIR>/cpp/build/libidevice++.a
                <INSTALL_DIR>/lib/idevice/libidevice++.a

            COMMAND ${CMAKE_COMMAND} -E copy_directory
                <SOURCE_DIR>/cpp/include
                <INSTALL_DIR>/include

        BUILD_IN_SOURCE 1
    )

    add_dependencies(palera1n idevice_ep)
endif()

set(IDEVICE_INCLUDE_DIR "${IDEVICE_INSTALL_DIR}/include")
set(IDEVICE_LIBRARY "${IDEVICE_INSTALL_DIR}/lib/idevice/libidevice++.a")
set(IDEVICE_FFI_LIBRARY "${IDEVICE_INSTALL_DIR}/lib/idevice/libidevice_ffi.a")

target_include_directories(palera1n PRIVATE ${IDEVICE_INCLUDE_DIR})
target_link_libraries(palera1n PRIVATE "${IDEVICE_LIBRARY}" "${IDEVICE_FFI_LIBRARY}")

if(MINGW)
    target_link_libraries(palera1n PRIVATE bcrypt userenv ntdll)
endif()

target_compile_definitions(palera1n PRIVATE LIBPLIST_STATIC)
