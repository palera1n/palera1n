cmake_minimum_required(VERSION 3.12)
project(palera1n VERSION 3.0)

include(FetchContent)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CROSS_HOST_TRIPLE "" CACHE STRING "Cross-compile target triple for Autotools (e.g., x86_64-linux-musl)")

option(WITH_GUI "Build with wxWidgets GUI" OFF)
option(WITH_STATIC "Build with static wxWidgets / deps" OFF)
option(WITH_TUI "Build with TUI" OFF)
option(WITH_RAMDISK "Build with ramdisk" ON)
option(WITH_BINPACK "Build with binpack" ON)
option(WITH_CIDERRAIN "Build with Ciderra1n instead of Openra1n" OFF)

# =========================
# BUILD REQUIREMENTS
# =========================

if(LINUX AND WITH_STATIC)
    if(NOT CROSS_HOST_TRIPLE MATCHES "musl" AND NOT CMAKE_C_COMPILER_TARGET MATCHES "musl" AND NOT CMAKE_C_COMPILER MATCHES "musl")
        message(FATAL_ERROR "Static builds on Linux require a musl toolchain. Please specify a musl cross-compiler or set -DCROSS_HOST_TRIPLE=...-linux-musl")
    endif()
endif()

if(MINGW AND NOT WITH_STATIC)
    message(FATAL_ERROR "MinGW builds must be static. Please configure with -DWITH_STATIC=1")
endif()

if(MINGW AND WITH_TUI)
    message(FATAL_ERROR "-DWITH_TUI=1 is not supported on MinGW builds.")
endif()

if(MINGW AND WITH_CIDERRAIN)
    message(FATAL_ERROR "-DWITH_CIDERRAIN=1 is not supported on MinGW builds.")
endif()

# =========================
# SETUP
# =========================

if(MINGW)
    set(DEPS_PREFIX /mingw64)
elseif(CMAKE_SYSROOT)
    set(DEPS_PREFIX "${CMAKE_SYSROOT}/usr")
else()
    set(DEPS_PREFIX "${CMAKE_BINARY_DIR}/sysroot/usr/local")
endif()

set(SYSROOT_FLAGS "")
if(CMAKE_SYSROOT)
    set(SYSROOT_FLAGS "--sysroot=${CMAKE_SYSROOT}")
endif()

set(AUTOTOOLS_BUILD_TRIPLE "x86_64-linux-gnu")

# =========================
# CMAKE DEPENDENCIES
# =========================

# wxWidgets
if(WITH_GUI)
    if(WITH_STATIC)
        set(wxBUILD_SHARED OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(
            wxWidgets
            GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
            GIT_SHALLOW ON
        )
        FetchContent_MakeAvailable(wxWidgets)
        set(WX_LIBRARIES wxcore wxbase wxnet)
    else()
        find_package(wxWidgets REQUIRED COMPONENTS core base net)
        include(${wxWidgets_USE_FILE})
        set(WX_LIBRARIES ${wxWidgets_LIBRARIES})
    endif()
endif()

# nlohmann_json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# =========================
# TARGET
# =========================

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.c"
    "src/*.h"
    "src/*.hpp"
)

if(APPLE AND WITH_GUI)
    set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/palera1n.icns")
    add_executable(palera1n MACOSX_BUNDLE ${SOURCES})
    target_sources(palera1n PRIVATE ${APP_ICON})
    set_source_files_properties(${APP_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_target_properties(palera1n PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist"
        MACOSX_BUNDLE_ICON_FILE palera1n.icns
    )
elseif(MINGW AND WITH_GUI)
    set(APP_MANIFEST_RC "${CMAKE_SOURCE_DIR}/resources/app.rc")
    add_executable(palera1n WIN32 ${SOURCES} ${APP_MANIFEST_RC})
else()
    add_executable(palera1n ${SOURCES})
endif()

target_include_directories(palera1n PRIVATE "${DEPS_PREFIX}/include")
target_link_directories(palera1n PRIVATE "${DEPS_PREFIX}/lib")
target_link_libraries(palera1n PRIVATE nlohmann_json::nlohmann_json)

if(WITH_RAMDISK)
    target_compile_definitions(palera1n PRIVATE WITH_RAMDISK)
endif()
if(WITH_BINPACK)
    target_compile_definitions(palera1n PRIVATE WITH_BINPACK)
endif()
if(WITH_GUI)
    target_link_libraries(palera1n PRIVATE ${WX_LIBRARIES})
    target_compile_definitions(palera1n PRIVATE WITH_GUI)
endif()
if(WITH_TUI)
    target_compile_definitions(palera1n PRIVATE WITH_TUI)
endif()
if(WITH_CIDERRAIN)
    target_compile_definitions(palera1n PRIVATE WITH_CIDERRAIN)
endif()

execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND git rev-parse --abbrev-ref --symbolic-full-name "@{u}"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

execute_process(
    COMMAND git remote get-url origin
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_URL
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

target_compile_definitions(palera1n PRIVATE
    GIT_HASH="${GIT_HASH}"
    GIT_BRANCH="${GIT_BRANCH}"
    GIT_URL="${GIT_URL}"
)

# =========================
# LINKS
# =========================

if(APPLE)
    find_library(IOKIT IOKit)
    find_library(COREFOUNDATION CoreFoundation)
    find_library(SECURITY Security)
    target_link_libraries(palera1n PRIVATE ${IOKIT} ${COREFOUNDATION} ${SECURITY})
endif()

if(WIN32)
    target_link_libraries(palera1n PRIVATE setupapi cfgmgr32)
endif()

# =========================
# NON-CMAKE DEPENDENCIES
# =========================

# ra1ns
if(WITH_CIDERRAIN)
    message(STATUS "Building with Ciderra1n support")

    # TODO: get rid
    execute_process(
        COMMAND patch -p1 -i ${CMAKE_SOURCE_DIR}/patches/liteusb/0001-cpp-compatible.patch
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/1stparty/libciderra1n/external/liteusb
        RESULT_VARIABLE LITEUSB_PATCH_RESULT
    )

    execute_process(
        COMMAND make CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/1stparty/libciderra1n
        RESULT_VARIABLE CIDERRA1N_BUILD_RESULT
    )
    if(NOT CIDERRA1N_BUILD_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to build Ciderra1n.")
    endif()
    set(CIDERRAIN_INCLUDE_DIR   "${CMAKE_SOURCE_DIR}/1stparty/libciderra1n/build/include")
    set(CIDERRAIN_LIB           "${CMAKE_SOURCE_DIR}/1stparty/libciderra1n/build/lib/ciderra1n/libciderra1n.a")
    target_include_directories(palera1n PRIVATE ${CIDERRAIN_INCLUDE_DIR})
    target_link_libraries(palera1n PRIVATE "${CIDERRAIN_LIB}")
    if(LINUX)
        set(LITEUSB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/1stparty/libciderra1n/external/liteusb/include")
        target_compile_definitions(palera1n PRIVATE LITEUSB_PLATFORM_LINUX)
        target_include_directories(palera1n PRIVATE ${LITEUSB_INCLUDE_DIR})
    endif()
else()
    message(STATUS "Building with Openra1n support")
    execute_process(
        COMMAND make CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/1stparty/libopenra1n
        RESULT_VARIABLE OPENRA1N_BUILD_RESULT
    )
    if(NOT OPENRA1N_BUILD_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to build Openra1n.")
    endif()
    set(OPENRA1N_INCLUDE_DIR   "${CMAKE_SOURCE_DIR}/1stparty/libopenra1n/build/include")
    set(OPENRA1N_LIB           "${CMAKE_SOURCE_DIR}/1stparty/libopenra1n/build/lib/openra1n/libopenra1n.a")
    target_include_directories(palera1n PRIVATE ${OPENRA1N_INCLUDE_DIR})
    target_link_libraries(palera1n PRIVATE "${OPENRA1N_LIB}")
endif()

# idevice
if(WITH_GUI OR WITH_TUI)
    ExternalProject_Add(idevice_ep
        GIT_REPOSITORY https://github.com/jkcoxson/idevice.git
        GIT_TAG v0.1.65
        PATCH_COMMAND patch --forward -p1 < "${CMAKE_SOURCE_DIR}/patches/idevice/0001-compile-for-usbmuxd-ring-only.patch" || true

        CONFIGURE_COMMAND ""

        BUILD_COMMAND
            ${CMAKE_COMMAND} -E chdir <SOURCE_DIR>/ffi
            cargo build --release --no-default-features --features "usbmuxd ring tcp"

        INSTALL_COMMAND
            ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/lib
            COMMAND ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/include
            COMMAND ${CMAKE_COMMAND} -E copy
                <SOURCE_DIR>/target/release/libidevice_ffi.a
                <INSTALL_DIR>/lib/idevice/libidevice_ffi.a
            COMMAND ${CMAKE_COMMAND} -E copy
                <SOURCE_DIR>/ffi/idevice.h
                <INSTALL_DIR>/include/idevice/idevice.h

        BUILD_IN_SOURCE 1
    )

    ExternalProject_Get_Property(idevice_ep install_dir)
    set(IDEVICE_LIBRARY "${install_dir}/lib/idevice/libidevice_ffi.a")
    set(IDEVICE_INCLUDE_DIR "${install_dir}/include")
    add_dependencies(palera1n idevice_ep)
    target_include_directories(palera1n PRIVATE ${IDEVICE_INCLUDE_DIR})
    target_link_libraries(palera1n PRIVATE "${IDEVICE_LIBRARY}")
endif()

if(APPLE)
    target_link_options(palera1n PRIVATE
        -Wl,-dead_strip
        -Wl,-x
    )
elseif(MINGW)
    target_link_options(palera1n PRIVATE
        -Wl,--gc-sections
        -Wl,--strip-all
    )
elseif(LINUX)
    target_link_options(palera1n PRIVATE
        -Wl,--gc-sections
        -Wl,--strip-all
    )
endif()

if(WITH_STATIC)
    find_program(PKG_CONFIG_EXECUTABLE NAMES pkg-config)

    set(BASE_CFLAGS "-g -I${DEPS_PREFIX}/include -L${DEPS_PREFIX}/lib")
    set(BASE_LDFLAGS "-g -L${DEPS_PREFIX}/lib")

    if(LINUX)
        set(PLATFORM_CFLAGS "-fdata-sections -ffunction-sections -D_FILE_OFFSET_BITS=64 -D_TIME_BITS=64")
        set(PLATFORM_LDFLAGS "-Wl,--gc-sections -fdata-sections -ffunction-sections")
    elseif(APPLE)
        set(PLATFORM_CFLAGS "")
        set(PLATFORM_LDFLAGS "-Wl,-dead_strip")
    else()
        set(PLATFORM_CFLAGS "")
        set(PLATFORM_LDFLAGS "")
    endif()

    set(SYSROOT_ENV_FLAG "")
    if(CMAKE_SYSROOT)
        set(SYSROOT_ENV_FLAG "--sysroot=${CMAKE_SYSROOT}")
    endif()

    set(PKG_ENV
        env
        "CC=${CMAKE_C_COMPILER}"
        "CXX=${CMAKE_CXX_COMPILER}"
        PKG_CONFIG=${PKG_CONFIG_EXECUTABLE}
        PKG_CONFIG_PATH=${DEPS_PREFIX}/lib/pkgconfig:${DEPS_PREFIX}/share/pkgconfig
        PKG_CONFIG_LIBDIR=${DEPS_PREFIX}/lib/pkgconfig:${DEPS_PREFIX}/share/pkgconfig

        CFLAGS=${BASE_CFLAGS}\ ${PLATFORM_CFLAGS}\ ${SYSROOT_ENV_FLAG}
        CPPFLAGS=-I${DEPS_PREFIX}/include
        LDFLAGS=${BASE_LDFLAGS}\ ${PLATFORM_LDFLAGS}\ ${SYSROOT_ENV_FLAG}

        C_INCLUDE_PATH=${DEPS_PREFIX}/include
        CPLUS_INCLUDE_PATH=${DEPS_PREFIX}/include

        mbedtls_INCLUDES=${DEPS_PREFIX}/include
        mbedtls_LIBDIR=${DEPS_PREFIX}/lib
        mbedtls_LIBS=-lmbedtls\ -lmbedx509\ -lmbedcrypto

        lt_cv_deplibs_check_method=pass_all
        lt_cv_file_magic_cmd=/usr/bin/file
        cross_compiling=yes
    )

    if(MINGW)
        set(IMD_AUTOTOOLS_ARGS --disable-static --enable-shared)
    else()
        set(IMD_AUTOTOOLS_ARGS --enable-static --disable-shared)
        if(CROSS_HOST_TRIPLE)
            list(APPEND IMD_AUTOTOOLS_ARGS "--host=${CROSS_HOST_TRIPLE}" "--build=${AUTOTOOLS_BUILD_TRIPLE}")
        endif()
        set(MBEDUTILS_ARGS --with-mbedtls)
    endif()

    if((LINUX OR MINGW) AND NOT WITH_CIDERRAIN)
        ExternalProject_Add(libusb_ep
            GIT_REPOSITORY https://github.com/libusb/libusb.git
            GIT_TAG v1.0.30
            UPDATE_COMMAND ""
            CONFIGURE_COMMAND
                ${PKG_ENV}
                sh -l ./autogen.sh --prefix=${DEPS_PREFIX} --disable-udev ${IMD_AUTOTOOLS_ARGS}
            BUILD_COMMAND make -j
            INSTALL_COMMAND make install
            BUILD_IN_SOURCE 1
        )

        add_dependencies(palera1n libusb_ep)
    endif()

    if(LINUX)
        ExternalProject_Add(ncurses_ep
            URL https://ftp.gnu.org/pub/gnu/ncurses/ncurses-6.4.tar.gz
            CONFIGURE_COMMAND
                ${PKG_ENV}
                sh <SOURCE_DIR>/configure
                    --prefix=${DEPS_PREFIX}
                    ${IMD_AUTOTOOLS_ARGS}
                    --disable-stripping
                    --enable-overwrite
                    --enable-widec
                    --with-default-terminfo-dir=${DEPS_PREFIX}/share/terminfo
                    --with-terminfo-dirs=${DEPS_PREFIX}/share/terminfo
            BUILD_COMMAND make -j
            BUILD_IN_SOURCE 1
            INSTALL_COMMAND
                ${CMAKE_COMMAND} -E env ${PKG_ENV}
                sh -c
                "make install TERMINFO=${DEPS_PREFIX}/share/terminfo && \
                ln -sf ${DEPS_PREFIX}/lib/libncursesw.a ${DEPS_PREFIX}/lib/libncurses.a && \
                ln -sf ${DEPS_PREFIX}/lib/libtinfow.a ${DEPS_PREFIX}/lib/libtinfo.a"
        )

        add_dependencies(palera1n ncurses_ep)
    endif()

    if(MINGW)
        if(NOT WITH_CIDERRAIN)
            target_link_libraries(palera1n PRIVATE
                usb-1.0
            )
        endif()

        set(DLLS
            libstdc++-6.dll
            libgcc_s_seh-1.dll
            libwinpthread-1.dll
        )

        if(NOT WITH_CIDERRAIN)
            list(APPEND DLLS
                libusb-1.0.dll
            )
        endif()

        execute_process(
            COMMAND cygpath -w "$ENV{MSYSTEM_PREFIX}/bin"
            OUTPUT_VARIABLE MSYSTEM_PREFIX_WIN
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        # all dlls must be in the same directory as the executable
        foreach(DLL ${DLLS})
            add_custom_command(TARGET palera1n POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${MSYSTEM_PREFIX_WIN}/${DLL}"
                    $<TARGET_FILE_DIR:palera1n>
            )
        endforeach()
    else()
        if(LINUX)
            target_compile_options(palera1n PRIVATE "-static")
            target_link_options(palera1n PRIVATE "-static")

            target_link_libraries(palera1n PRIVATE
                "${DEPS_PREFIX}/lib/libncurses.a"
            )
            if(NOT WITH_CIDERRAIN)
                target_link_libraries(palera1n PRIVATE
                    "${DEPS_PREFIX}/lib/libusb-1.0.a"
                )
            endif()
        elseif(APPLE)
            if(WITH_TUI)
                find_package(Curses REQUIRED)
                target_include_directories(palera1n PRIVATE ${CURSES_INCLUDE_DIRS})
                target_link_libraries(palera1n PRIVATE ${CURSES_LIBRARIES})
            endif()
        endif()
    endif()
else()
    find_package(Curses REQUIRED)
    target_include_directories(palera1n PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(palera1n PRIVATE ${CURSES_LIBRARIES})

    if(NOT APPLE AND NOT CIDERRAIN)
        pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
        target_include_directories(palera1n PRIVATE ${LIBUSB_INCLUDE_DIRS})
        target_link_libraries(palera1n PRIVATE ${LIBUSB_LIBRARIES})
        target_compile_options(palera1n PRIVATE ${LIBUSB_CFLAGS_OTHER})
    endif()
endif()
