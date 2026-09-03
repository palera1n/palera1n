cmake_minimum_required(VERSION 3.12)
project(palera1n VERSION 3.0)

include(FetchContent)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if(NOT IOS)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11")
else()
set(CMAKE_OSX_DEPLOYMENT_TARGET "9.0")
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CROSS_HOST_TRIPLE "" CACHE STRING "Cross-compile target triple for Autotools (e.g., x86_64-linux-musl)")

option(WITH_GUI "Build with wxWidgets GUI" OFF)
option(WITH_STATIC "Build with static wxWidgets / deps" OFF)
option(WITH_TUI "Build with TUI" OFF)
option(WITH_RAMDISK "Build with ramdisk" ON)
option(WITH_BINPACK "Build with binpack" ON)
option(WITH_CIDERRAIN "Build with Ciderra1n instead of Openra1n" OFF)

# =========================
# MARK: BUILD REQUIREMENTS
# =========================

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildRequirements.cmake")

# =========================
# MARK: SETUP
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
# MARK: CMAKE DEPENDENCIES
# =========================

set(wxBUILD_INSTALL OFF CACHE BOOL "" FORCE)
set(JSON_Install OFF CACHE BOOL "" FORCE)

# wxWidgets
if(WITH_GUI)
    if(WITH_STATIC)
        set(wxBUILD_SHARED OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(
            wxWidgets
            GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
            GIT_TAG v3.3.3
            PATCH_COMMAND
                patch --forward --ignore-whitespace -p1 < "${CMAKE_SOURCE_DIR}/patches/wxWidgets/0001-cpp11-support.patch" || true
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
# MARK: TARGET
# =========================

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.c"
    "src/*.h"
    "src/*.hpp"
)

if((APPLE AND NOT IOS) AND WITH_GUI)
    set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/palera1n.icns")
    add_executable(palera1n MACOSX_BUNDLE ${SOURCES})
    target_sources(palera1n PRIVATE ${APP_ICON})
    set_source_files_properties(${APP_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_target_properties(palera1n PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist"
        MACOSX_BUNDLE_ICON_FILE palera1n.icns
    )
elseif(IOS)
    add_executable(palera1n MACOSX_BUNDLE ${SOURCES})

    set_target_properties(palera1n PROPERTIES
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "in.palera.palera1n"
        XCODE_ATTRIBUTE_AD_HOC_CODE_SIGNING_ALLOWED "YES"
        XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "YES"
        XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "YES"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-"
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_SOURCE_DIR}/resources/entitlements.xml"
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

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Git.cmake")

# =========================
# MARK: LINKS
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
# MARK: NON-CMAKE DEPENDENCIES (LOCAL)
# =========================

# ra1ns
if(WITH_CIDERRAIN)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Ciderra1n.cmake")
else()
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Openra1n.cmake")
endif()

# idevice
if(WITH_GUI OR WITH_TUI)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Idevice.cmake")
endif()

# =========================
# MARK: NON-CMAKE DEPENDENCIES
# =========================

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
    if(WITH_TUI)
        find_package(Curses REQUIRED)
        target_include_directories(palera1n PRIVATE ${CURSES_INCLUDE_DIRS})
        target_link_libraries(palera1n PRIVATE ${CURSES_LIBRARIES})
    endif()

    if(NOT APPLE AND NOT CIDERRAIN)
        pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
        target_include_directories(palera1n PRIVATE ${LIBUSB_INCLUDE_DIRS})
        target_link_libraries(palera1n PRIVATE ${LIBUSB_LIBRARIES})
        target_compile_options(palera1n PRIVATE ${LIBUSB_CFLAGS_OTHER})
    endif()
endif()

# =========================
# MARK: STRIP FLAGS
# =========================

if(CMAKE_BUILD_TYPE STREQUAL "Release")
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
endif()

# =========================
# MARK: INSTALL
# =========================

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildInstall.cmake")

