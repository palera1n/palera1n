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

if(LINUX AND WITH_STATIC)
    if(NOT CROSS_HOST_TRIPLE MATCHES "musl" AND NOT CMAKE_C_COMPILER_TARGET MATCHES "musl" AND NOT CMAKE_C_COMPILER MATCHES "musl")
        message(FATAL_ERROR "Static builds on Linux require a musl toolchain. Please specify a musl cross-compiler or set -DCROSS_HOST_TRIPLE=...-linux-musl")
    endif()
endif()

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

# LZ4
set(LZ4_BUILD_CLI OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    lz4
    GIT_REPOSITORY https://github.com/lz4/lz4.git
    GIT_TAG v1.10.0
    SOURCE_SUBDIR build/cmake
)
FetchContent_MakeAvailable(lz4)

# =========================
# TARGET DEFINITION
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
target_link_libraries(palera1n PRIVATE nlohmann_json::nlohmann_json lz4_static)

if(WITH_RAMDISK)
    target_compile_definitions(palera1n PRIVATE WITH_RAMDISK)
endif()
if(WITH_BINPACK)
    target_compile_definitions(palera1n PRIVATE WITH_BINPACK)
endif()

# =========================
# APPLE FRAMEWORKS
# =========================
if(APPLE)
    find_library(IOKIT IOKit)
    find_library(COREFOUNDATION CoreFoundation)
    find_library(SECURITY Security)
    target_link_libraries(palera1n PRIVATE ${IOKIT} ${COREFOUNDATION} ${SECURITY})
endif()

# =========================
# GUI
# =========================
if(WITH_GUI)
    target_link_libraries(palera1n PRIVATE ${WX_LIBRARIES})
    target_compile_definitions(palera1n PRIVATE WITH_GUI)
endif()
if(WITH_TUI)
    target_compile_definitions(palera1n PRIVATE WITH_TUI)
endif()

# =========================
# libimobiledevice
# =========================
if(WITH_STATIC)
    find_program(PKG_CONFIG_EXECUTABLE NAMES pkg-config)

    set(PKG_ENV
        env
        "CC=${CMAKE_C_COMPILER}"
        "CXX=${CMAKE_CXX_COMPILER}"
        PKG_CONFIG=${PKG_CONFIG_EXECUTABLE}
        PKG_CONFIG_PATH=${DEPS_PREFIX}/lib/pkgconfig:${DEPS_PREFIX}/share/pkgconfig
        PKG_CONFIG_LIBDIR=${DEPS_PREFIX}/lib/pkgconfig:${DEPS_PREFIX}/share/pkgconfig

        CFLAGS=-g\ -fdata-sections\ -ffunction-sections\ -I${DEPS_PREFIX}/include\ -L${DEPS_PREFIX}/lib\ -D_FILE_OFFSET_BITS=64\ -D_TIME_BITS=64\ --sysroot=${CMAKE_SYSROOT}
        CPPFLAGS=-I${DEPS_PREFIX}/include
        LDFLAGS=-g\ -Wl,--gc-sections\ -fdata-sections\ -ffunction-sections\ -I${DEPS_PREFIX}/include\ -L${DEPS_PREFIX}/lib --sysroot=${CMAKE_SYSROOT}

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

    if(MINGW)
        ExternalProject_Add(libwdi_ep
            GIT_REPOSITORY https://github.com/pbatard/libwdi.git
            GIT_TAG v1.5.1
            UPDATE_COMMAND ""
            CONFIGURE_COMMAND
                ${PKG_ENV}
                sh -lc "./bootstrap.sh && ./configure --prefix=${DEPS_PREFIX} --build=x86_64-w64-mingw32 --host=x86_64-w64-mingw32 --disable-32bit --with-wdfver=1011 --with-libusbk=${CMAKE_SOURCE_DIR}/libusbK/bin"
            BUILD_COMMAND make -j
            INSTALL_COMMAND make install
            BUILD_IN_SOURCE 1
        )
    endif()

    if(NOT MINGW)
        set(MBEDTLS_PATCH_STEPS PATCH_COMMAND patch -p1 < ${CMAKE_SOURCE_DIR}/patches/mbedtls/0001-Allow-empty-x509-cert-issuer.patch)
        if(CROSS_HOST_TRIPLE MATCHES "i486-linux-musl")
            list(APPEND MBEDTLS_PATCH_STEPS COMMAND sed -i "/define MBEDTLS_AESNI_C/d" include/mbedtls/mbedtls_config.h)
        endif()

        ExternalProject_Add(mbedtls_ep
            GIT_REPOSITORY https://github.com/Mbed-TLS/mbedtls.git
            GIT_TAG mbedtls-3.5.2
            UPDATE_COMMAND ""
            ${MBEDTLS_PATCH_STEPS}
            CMAKE_ARGS
                -DCMAKE_INSTALL_PREFIX=${DEPS_PREFIX}
                -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                -DCMAKE_SYSROOT=${CMAKE_SYSROOT}
                -DENABLE_TESTING=OFF
                -DENABLE_PROGRAMS=OFF
                -DBUILD_SHARED_LIBS=OFF
                -DENABLE_ASSEMBLY=OFF
            INSTALL_DIR ${DEPS_PREFIX}
        )
    endif()

    if(LINUX OR MINGW)
        ExternalProject_Add(libusb_ep
            GIT_REPOSITORY https://github.com/libusb/libusb.git
            GIT_TAG v1.0.27
            UPDATE_COMMAND ""
            CONFIGURE_COMMAND
                ${PKG_ENV}
                sh -l ./autogen.sh --prefix=${DEPS_PREFIX} --disable-udev ${IMD_AUTOTOOLS_ARGS}
            BUILD_COMMAND make -j
            INSTALL_COMMAND make install
            BUILD_IN_SOURCE 1
        )
    endif()

    if(LINUX)
        ExternalProject_Add(ncurses_ep
            URL https://ftp.gnu.org/pub/gnu/ncurses/ncurses-6.4.tar.gz
            CONFIGURE_COMMAND
                ${PKG_ENV}
                sh -l <SOURCE_DIR>/configure --prefix=${DEPS_PREFIX} ${IMD_AUTOTOOLS_ARGS} --disable-stripping
            BUILD_COMMAND make -j
            BUILD_IN_SOURCE 1
            INSTALL_COMMAND make install
        )
    endif()

    if(LINUX)
        ExternalProject_Add(readline_ep
            URL https://ftp.gnu.org/gnu/readline/readline-8.2.tar.gz
            CONFIGURE_COMMAND
                ${PKG_ENV}
                sh -l <SOURCE_DIR>/configure --prefix=${DEPS_PREFIX} ${IMD_AUTOTOOLS_ARGS}
            BUILD_COMMAND make -j
            INSTALL_COMMAND make install
            BUILD_IN_SOURCE 1
            DEPENDS ncurses_ep
        )
    endif()

    ExternalProject_Add(libplist_ep
        GIT_REPOSITORY https://github.com/libimobiledevice/libplist.git
        GIT_TAG 2117b8fdb6b4096455bd2041a63e59a028120136
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND
            ${PKG_ENV}
            sh -l ./autogen.sh --prefix=${DEPS_PREFIX} ${IMD_AUTOTOOLS_ARGS}
        BUILD_COMMAND make -j
        INSTALL_COMMAND make install
        BUILD_IN_SOURCE 1
    )

    ExternalProject_Add(limd_glue_ep
        GIT_REPOSITORY https://github.com/libimobiledevice/libimobiledevice-glue.git
        GIT_TAG 362f7848ac89b74d9dd113b38b51ecb601f76094
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND
            ${PKG_ENV}
            sh -l ./autogen.sh --prefix=${DEPS_PREFIX} ${IMD_AUTOTOOLS_ARGS}
        BUILD_COMMAND make -j
        INSTALL_COMMAND make install
        BUILD_IN_SOURCE 1
        DEPENDS libplist_ep
    )

    ExternalProject_Add(libusbmuxd_ep
        GIT_REPOSITORY https://github.com/libimobiledevice/libusbmuxd.git
        GIT_TAG a7f0543fb1ecb20ac7121c0fd77297200e0e43fc
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND
            ${PKG_ENV}
            sh -l ./autogen.sh --prefix=${DEPS_PREFIX} ${IMD_AUTOTOOLS_ARGS} --without-systemd
        BUILD_COMMAND make -j
        INSTALL_COMMAND make install
        BUILD_IN_SOURCE 1
        DEPENDS libplist_ep limd_glue_ep
    )

    set(LIBIMOBILEDEVICE_DEPS libplist_ep libusbmuxd_ep limd_glue_ep)
    if(NOT MINGW)
        list(APPEND LIBIMOBILEDEVICE_DEPS mbedtls_ep)
    endif()
    if(LINUX OR MINGW)
        list(APPEND LIBIMOBILEDEVICE_DEPS libusb_ep)
    endif()
    if(LINUX)
        list(APPEND LIBIMOBILEDEVICE_DEPS readline_ep)
    endif()

    ExternalProject_Add(libimobiledevice_ep
        GIT_REPOSITORY https://github.com/libimobiledevice/libimobiledevice.git
        GIT_TAG ed0d66d0341562731bb19928dfe48155509fa7a7
        UPDATE_COMMAND ""
        PATCH_COMMAND patch -p1 < "${CMAKE_SOURCE_DIR}/patches/libimobiledevice/0001-include-windows-header.patch"
        CONFIGURE_COMMAND
            ${PKG_ENV}
            sh -l ./autogen.sh --prefix=${DEPS_PREFIX} ${MBEDUTILS_ARGS} ${IMD_AUTOTOOLS_ARGS} --disable-wireless-pairing
        BUILD_COMMAND make -j
        INSTALL_COMMAND make install
        BUILD_IN_SOURCE 1
        DEPENDS ${LIBIMOBILEDEVICE_DEPS}
    )

    file(WRITE "${CMAKE_BINARY_DIR}/fix_tools_makefile.cmake"
        "file(WRITE \"${CMAKE_BINARY_DIR}/libimobiledevice_ep-prefix/src/libimobiledevice_ep/tools/Makefile\" \"all:\\ninstall:\\n\")\n"
    )

    ExternalProject_Add_Step(libimobiledevice_ep fix_tools_makefile
        COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/fix_tools_makefile.cmake"
        DEPENDEES configure
        DEPENDERS build
    )

    if(LINUX)
        set(LIBIRECOVERY_LIBS LIBS=-lncurses)
    endif()

    ExternalProject_Add(libirecovery_ep
        GIT_REPOSITORY https://github.com/libimobiledevice/libirecovery.git
        GIT_TAG 7ce02c347b7c26e59498e6af31c9da51018d0fa1
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND
            ${PKG_ENV}
            sh -l ./autogen.sh --prefix=${DEPS_PREFIX} ${IMD_AUTOTOOLS_ARGS} ${LIBIRECOVERY_LIBS}
        BUILD_COMMAND make -j
        INSTALL_COMMAND make install
        BUILD_IN_SOURCE 1
        DEPENDS ${LIBIMOBILEDEVICE_DEPS}
    )

    add_dependencies(palera1n
        libplist_ep
        limd_glue_ep
        libusbmuxd_ep
        libimobiledevice_ep
        libirecovery_ep
    )

    if(NOT MINGW)
        add_dependencies(palera1n mbedtls_ep)
    else()
        add_dependencies(palera1n libwdi_ep)
    endif()

    if(LINUX)
        add_dependencies(palera1n readline_ep)
    endif()

    if(MINGW)
        target_link_libraries(palera1n PRIVATE
            imobiledevice-1.0
            irecovery-1.0
            usbmuxd-2.0
            plist-2.0
            imobiledevice-glue-1.0
            usb-1.0
            libwdi
        )

        set(DLLS
            libcrypto-3-x64.dll
            libssl-3-x64.dll
            libstdc++-6.dll
            libgcc_s_seh-1.dll
            libwinpthread-1.dll
            libimobiledevice-1.0.dll
            libimobiledevice-glue-1.0.dll
            libirecovery-1.0.dll
            libplist-2.0.dll
            libusbmuxd-2.0.dll
            libusb-1.0.dll
            libwdi.dll
        )

        execute_process(
            COMMAND cygpath -w "$ENV{MSYSTEM_PREFIX}/bin"
            OUTPUT_VARIABLE MSYSTEM_PREFIX_WIN
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        foreach(DLL ${DLLS})
            add_custom_command(TARGET palera1n POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${MSYSTEM_PREFIX_WIN}/${DLL}"
                    $<TARGET_FILE_DIR:palera1n>
            )
        endforeach()
    else()
        target_link_libraries(palera1n PRIVATE
            "${DEPS_PREFIX}/lib/libimobiledevice-1.0.a"
            "${DEPS_PREFIX}/lib/libirecovery-1.0.a"
            "${DEPS_PREFIX}/lib/libusbmuxd-2.0.a"
            "${DEPS_PREFIX}/lib/libplist-2.0.a"
            "${DEPS_PREFIX}/lib/libimobiledevice-glue-1.0.a"
            "${DEPS_PREFIX}/lib/libmbedtls.a"
            "${DEPS_PREFIX}/lib/libmbedx509.a"
            "${DEPS_PREFIX}/lib/libmbedcrypto.a"
        )

        if(LINUX)
            target_compile_options(palera1n PRIVATE "-static")
            target_link_options(palera1n PRIVATE "-static")

            target_link_libraries(palera1n PRIVATE
                "${DEPS_PREFIX}/lib/libusb-1.0.a"
                "${DEPS_PREFIX}/lib/libreadline.a"
                "${DEPS_PREFIX}/lib/libncurses.a"
            )
        elseif(APPLE)
            if(WITH_TUI)
                find_package(Curses REQUIRED)
                target_include_directories(palera1n PRIVATE ${CURSES_INCLUDE_DIRS})
                target_link_libraries(palera1n PRIVATE ${CURSES_LIBRARIES})
            endif()
        endif()
    endif()
else()
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(IMD REQUIRED libimobiledevice-1.0)
    pkg_check_modules(IRECOVERY REQUIRED libirecovery-1.0)
    pkg_check_modules(PLIST REQUIRED libplist-2.0)
    pkg_check_modules(USBMUXD REQUIRED libusbmuxd-2.0)

    target_include_directories(palera1n PRIVATE
        ${IMD_INCLUDE_DIRS}
        ${IRECOVERY_INCLUDE_DIRS}
        ${PLIST_INCLUDE_DIRS}
        ${USBMUXD_INCLUDE_DIRS}
    )

    target_link_libraries(palera1n PRIVATE
        ${IMD_LIBRARIES}
        ${IRECOVERY_LIBRARIES}
        ${PLIST_LIBRARIES}
        ${USBMUXD_LIBRARIES}
    )

    target_compile_options(palera1n PRIVATE
        ${IMD_CFLAGS_OTHER}
        ${IRECOVERY_CFLAGS_OTHER}
        ${PLIST_CFLAGS_OTHER}
        ${USBMUXD_CFLAGS_OTHER}
    )

    find_package(Curses REQUIRED)
    target_include_directories(palera1n PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(palera1n PRIVATE ${CURSES_LIBRARIES})

    if(NOT APPLE)
        pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
        target_include_directories(palera1n PRIVATE ${LIBUSB_INCLUDE_DIRS})
        target_link_libraries(palera1n PRIVATE ${LIBUSB_LIBRARIES})
        target_compile_options(palera1n PRIVATE ${LIBUSB_CFLAGS_OTHER})
    endif()
endif()
