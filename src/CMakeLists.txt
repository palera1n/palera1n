#execute_process(COMMAND sh -c "cd ../dep_root && sh ../download_deps.sh")
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")


#set(SRC ${CMAKE_CURRENT_SOURCE_DIR})
#set(DEP ${SRC}/../dep_root)



set(CHECKRA1N_VERSION 0.1337.1)
if( ${LDID})
    set(LDID ldid)
endif()
if( ${LIPO})
    set(LIPO lipo)
endif()

if(APPLE)
    set(CHECKRA1N_FILE_NAME checkra1n-macos)
else()
    set(CHECKRA1N_FILE_NAME checkra1n-linux-${CMAKE_SYSTEM_PROCESSOR})
endif()

add_executable(palera1n
        binpack.dmg.c
        checkra1n-kpf-pongo.c
        checkra1n.c
        devhelper.c
        dfuhelper.c
        exec_checkra1n.c
        lock_vars.c
        lockdown_helper.c
        log.c
        main.c
        optparse.c
        override_file.c
        pongo_helper.c
        ramdisk.dmg.c
        ../include/kerninfo.h
        ../include/palerain.h
        ../include/ANSI-color-codes.h
        ../include/tui.h
        ../include/xxd-embedded.h
        )


# adds dev mode files
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    get_property(MY_TARGET_SOURCES TARGET palera1n PROPERTY SOURCES)
    list(APPEND MY_TARGET_SOURCES tui_main.c
            tui_screen_enter_dfu.c
            tui_screen_enter_recovery.c
            tui_screen_jailbreak.c
            tui_screen_main.c
            tui_screen_options.c)
    set_property(TARGET palera1n PROPERTY SOURCES ${MY_TARGET_SOURCES})


elseif(CMAKE_BUILD_TYPE STREQUAL "ASAN")
    get_property(MY_TARGET_SOURCES TARGET palera1n PROPERTY SOURCES)
    list(APPEND MY_TARGET_SOURCES tui_main.c
            tui_screen_enter_dfu.c
            tui_screen_enter_recovery.c
            tui_screen_jailbreak.c
            tui_screen_main.c
            tui_screen_options.c)
    set_property(TARGET palera1n PROPERTY SOURCES ${MY_TARGET_SOURCES})
    add_compile_options(-fsanitize=address,undefined -fsanitize-address-use-after-return=runtime)
elseif(CMAKE_BUILD_TYPE STREQUAL "TSAN")
get_property(MY_TARGET_SOURCES TARGET palera1n PROPERTY SOURCES)
    list(APPEND MY_TARGET_SOURCES tui_main.c
            tui_screen_enter_dfu.c
            tui_screen_enter_recovery.c
            tui_screen_jailbreak.c
            tui_screen_main.c
            tui_screen_options.c)
    set_property(TARGET palera1n PROPERTY SOURCES ${MY_TARGET_SOURCES})
    add_compile_options(-fsanitize=thread,undefined)
else()
    set(CMAKE_BUILD_TYPE "Release")
endif()

#message("Testing for Clang/LLVM")
#string(TOLOWER CMAKE_C_COMPILER_ID COMPILER)

#if(CMAKE_C_COMPILER_ID MATCHES "AppleClang" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
#    message("Using clang/llvm! Enabling -flto=full! Please make sure clang is up to date, or else compile will fail")
#    add_compile_options(-flto=full)
#else()
#    message("Not using clang/llvm! Using normal lto/no lto!")
#    add_compile_options(-flto)
#endif()



#add_custom_command(TARGET palera1n
#        COMMAND sh -c "curl -LO https://assets.checkra.in/downloads/preview/${CHECKRA1N_VERSION}/${CHECKRA1N_FILE_NAME} && chmod 755 ${CHECKRA1N_FILE_NAME}"
#        )
#
#if(APPLE)
#    add_custom_command(TARGET palera1n
#            COMMAND sh -c "ldid -Scheckra1n.plist checkra1n"
#            )
#    add_custom_command(TARGET palera1n
#            COMMAND lipo -thin ${CMAKE_SYSTEM_PROCESSOR} -output checkra1n checkra1n-macos
#            )
#    if(CFLAGS MATCHES "version-min=")
#        add_custom_command(TARGET palera1n
#                COMMAND sh -c "vtool -remove-build-version 1 -replace -output checkra1n checkra1n && vtool -set-build-version $(FAKE_PLATFORM) $(FAKE_MINOS) $(FAKE_SDK) -replace -output checkra1n checkra1n && install_name_tool -change /System/Library/Frameworks/IOKit.framework/Versions/A/IOKit /System/Library/Frameworks/IOKit.framework/IOKit checkra1n && install_name_tool -change /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation /System/Library/Frameworks/CoreFoundation.framework/CoreFoundation checkra1n && ${LDID} -Susb.xml checkra1n"
#                )
#    endif()
#else()
#    execute_process(
#            COMMAND cp ${CHECKRA1N_FILE_NAME} ../src/checkra1n
#    )
#endif()

#ifneq (,$(findstring arm64, $(CFLAGS)))
#$(LIPO) -thin arm64 -output checkra1n checkra1n-macos
#else ifneq (,$(findstring x86_64, $(CFLAGS)))
#$(LIPO) -thin x86_64 -output checkra1n checkra1n-macos
#else ifeq ($(shell arch),i386)
#$(LIPO) -thin x86_64 -output checkra1n checkra1n-macos
#else
#$(LIPO) -thin $(shell arch) -output checkra1n checkra1n-

if(APPLE)
    if(${LIBUSB})
        target_compile_definitions(palera1n PRIVATE -DUSE_LIBUSB)
        get_property(MY_TARGET_SOURCES TARGET palera1n PROPERTY SOURCES)
        list(APPEND MY_TARGET_SOURCES
                usb_libusb.c)
        set_property(TARGET palera1n PROPERTY SOURCES ${MY_TARGET_SOURCES})
        target_link_libraries(palera1n
                ${DEP}/lib/libusb-1.0.a
                )
    else()
        get_property(MY_TARGET_SOURCES TARGET palera1n PROPERTY SOURCES)
        list(APPEND MY_TARGET_SOURCES
                usb_iokit.c)
        set_property(TARGET palera1n PROPERTY SOURCES ${MY_TARGET_SOURCES})
    endif()

    if(IOS AND CMAKE_C_FLAGS MATCHES "arm64")
        add_custom_command(
                COMMAND sh -c "curl -Lo checkra1n-macos https://assets.checkra.in/downloads/preview/0.1337.1/${CHECKRA1N_FILE_NAME} "
                COMMAND lipo -thin arm64 -output checkra1n checkra1n-macos
                COMMAND sh -c "vtool -remove-build-version 1 -replace -output checkra1n checkra1n && vtool -set-build-version $(FAKE_PLATFORM) $(FAKE_MINOS) $(FAKE_SDK) -replace -output checkra1n checkra1n && install_name_tool -change /System/Library/Frameworks/IOKit.framework/Versions/A/IOKit /System/Library/Frameworks/IOKit.framework/IOKit checkra1n && install_name_tool -change /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation /System/Library/Frameworks/CoreFoundation.framework/CoreFoundation checkra1n && ${LDID} -Susb.xml checkra1n"
                COMMAND sh -c "curl -Lo binpack.dmg https://cdn.nickchan.lol/palera1n/c-rewrite/deps/binpack.dmg"
                COMMAND sh -c "curl -Lo ramdisk.dmg https://cdn.nickchan.lol/palera1n/c-rewrite/deps/ramdisk.dmg"
                COMMAND sh -c "curl -Lo checkra1n-kpf-pongo https://github.com/guacaplushy/checkp4le/raw/main/binaries/checkra1n-kpf-pongo"
                COMMAND pwd
                COMMAND xxd -iC checkra1n checkra1n.c
                COMMAND xxd -iC binpack.dmg binpack.dmg.c
                COMMAND xxd -iC ramdisk.dmg ramdisk.dmg.c
                COMMAND xxd -iC checkra1n-kpf-pongo checkra1n-kpf-pongo.c
                OUTPUT checkra1n.c checkra1n-kpf-pongo.c ramdisk.dmg.c binpack.dmg.c binpack.dmg ramdisk.dmg checkra1n-kpf-pongo
        )
    elseif(CMAKE_C_FLAGS MATCHES "arm64")
        add_custom_command(
                COMMAND sh -c "curl -Lo checkra1n-macos https://assets.checkra.in/downloads/preview/0.1337.1/${CHECKRA1N_FILE_NAME} "
                COMMAND lipo -thin arm64 -output checkra1n checkra1n-macos
                COMMAND sh -c "curl -Lo binpack.dmg https://cdn.nickchan.lol/palera1n/c-rewrite/deps/binpack.dmg"
                COMMAND sh -c "curl -Lo ramdisk.dmg https://cdn.nickchan.lol/palera1n/c-rewrite/deps/ramdisk.dmg"
                COMMAND sh -c "curl -Lo checkra1n-kpf-pongo https://github.com/guacaplushy/checkp4le/raw/main/binaries/checkra1n-kpf-pongo"
                COMMAND pwd
                COMMAND xxd -iC checkra1n checkra1n.c
                COMMAND xxd -iC binpack.dmg binpack.dmg.c
                COMMAND xxd -iC ramdisk.dmg ramdisk.dmg.c
                COMMAND xxd -iC checkra1n-kpf-pongo checkra1n-kpf-pongo.c
                OUTPUT checkra1n.c checkra1n-kpf-pongo.c ramdisk.dmg.c binpack.dmg.c binpack.dmg ramdisk.dmg checkra1n-kpf-pongo
        )
    else()
        add_custom_command(
                COMMAND sh -c "curl -Lo checkra1n-macos https://assets.checkra.in/downloads/preview/0.1337.1/${CHECKRA1N_FILE_NAME} "
                COMMAND lipo -thin x86_64 -output checkra1n checkra1n-macos
                COMMAND sh -c "curl -Lo binpack.dmg https://cdn.nickchan.lol/palera1n/c-rewrite/deps/binpack.dmg"
                COMMAND sh -c "curl -Lo ramdisk.dmg https://cdn.nickchan.lol/palera1n/c-rewrite/deps/ramdisk.dmg"
                COMMAND sh -c "curl -Lo checkra1n-kpf-pongo https://github.com/guacaplushy/checkp4le/raw/main/binaries/checkra1n-kpf-pongo"
                COMMAND pwd
                COMMAND xxd -iC checkra1n checkra1n.c
                COMMAND xxd -iC binpack.dmg binpack.dmg.c
                COMMAND xxd -iC ramdisk.dmg ramdisk.dmg.c
                COMMAND xxd -iC checkra1n-kpf-pongo checkra1n-kpf-pongo.c
                OUTPUT checkra1n.c checkra1n-kpf-pongo.c ramdisk.dmg.c binpack.dmg.c binpack.dmg ramdisk.dmg checkra1n-kpf-pongo
        )
    endif()
else()
    add_custom_command(
            COMMAND sh -c "curl -Lo checkra1n https://assets.checkra.in/downloads/preview/0.1337.1/${CHECKRA1N_FILE_NAME} "
            COMMAND sh -c "curl -Lo binpack.dmg https://cdn.nickchan.lol/palera1n/c-rewrite/deps/binpack.dmg"
            COMMAND sh -c "curl -Lo ramdisk.dmg https://cdn.nickchan.lol/palera1n/c-rewrite/deps/ramdisk.dmg"
            COMMAND sh -c "curl -Lo checkra1n-kpf-pongo https://github.com/guacaplushy/checkp4le/raw/main/binaries/checkra1n-kpf-pongo"
            COMMAND pwd
            COMMAND xxd -iC checkra1n checkra1n.c
            COMMAND xxd -iC binpack.dmg binpack.dmg.c
            COMMAND xxd -iC ramdisk.dmg ramdisk.dmg.c
            COMMAND xxd -iC checkra1n-kpf-pongo checkra1n-kpf-pongo.c
            OUTPUT checkra1n.c checkra1n-kpf-pongo.c ramdisk.dmg.c binpack.dmg.c binpack.dmg ramdisk.dmg checkra1n-kpf-pongo
    )
    target_compile_definitions(palera1n PRIVATE -DUSE_LIBUSB)
    get_property(MY_TARGET_SOURCES TARGET palera1n PROPERTY SOURCES)
    list(APPEND MY_TARGET_SOURCES
            usb_libusb.c)
    set_property(TARGET palera1n PROPERTY SOURCES ${MY_TARGET_SOURCES})
    target_link_libraries(palera1n
            ${DEP}/lib/libusb-1.0.a
            )
    target_compile_definitions(palera1n PRIVATE -DUSE_LIBUSB)
endif()

include_directories(
        ../include
        ${SRC}
)


target_compile_definitions(palera1n PRIVATE -DPALERAIN_VERSION=\"2.0.0\")

target_include_directories(palera1n PRIVATE
        ${SRC}/include
        ${SRC}
        ${DEP}/include
        )

#if(${STATIC})
    target_link_libraries(palera1n
            ${DEP}/lib/libimobiledevice-1.0.a
            ${DEP}/lib/libirecovery-1.0.a
            ${DEP}/lib/libusbmuxd-2.0.a
            ${DEP}/lib/libimobiledevice-glue-1.0.a
            ${DEP}/lib/libplist-2.0.a
            ${DEP}/lib/libmbedtls.a
            ${DEP}/lib/libmbedcrypto.a
            ${DEP}/lib/libmbedx509.a
            ${DEP}/lib/libreadline.a
            )

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
if(NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()
#else()
#    find_library(LIBIMOBILEDEVICE libimobiledevice)
#    if (NOT CORE_FOUNDATION)
#        message(FATAL_ERROR "libimobiledevice not found")
#    endif()
#    target_link_libraries(palera1n ${LIBIMOBILEDEVICE})
#
#    find_library(LIR libirecovery)
#    if (NOT SYSCONF)
#        message(FATAL_ERROR "libirecovery not found")
#    endif()
#    target_link_libraries(palera1n ${LIR})
#
#    find_library(USBMUXD usbmuxd)
#    if (NOT SEC)
#        message(FATAL_ERROR "usbmuxd not found")
#    endif()
#    target_link_libraries(palera1n ${USBMUXD})
#
#    find_library(GLUE libimobiledevice-glue)
#    if (NOT IOKIT)
#        message(FATAL_ERROR "libimobiledevice-glue not found")
#    endif()
#    target_link_libraries(palera1n ${GLUE})
#
##    target_link_libraries(palera1n
##            imobiledevice-1.0
##            irecovery-1.0
##            usbmuxd-2.0
##            imobiledevice-glue-1.0
##            plist-2.0
##            mbedtls
##            mbedcrypto
##            mbedx509
##            readline
##            usb-1.0
##            )
#endif()
#if(APPLE)
#    add_compile_options(palera1n
#            -framework CoreFoundation
#            -framework SystemConfiguration
#            -framework IOKit
#            -framework Security
#            )
#endif()

if(APPLE)
    find_library(CORE_FOUNDATION CoreFoundation)
    if (NOT CORE_FOUNDATION)
        message(FATAL_ERROR "CoreFoundation not found")
    endif()
    target_link_libraries(palera1n ${CORE_FOUNDATION})

    find_library(SYSCONF SystemConfiguration)
    if (NOT SYSCONF)
        message(FATAL_ERROR "SystemConfiguration not found")
    endif()
    target_link_libraries(palera1n ${SYSCONF})

    find_library(SEC Security)
    if (NOT SEC)
        message(FATAL_ERROR "Security not found")
    endif()
    target_link_libraries(palera1n ${SEC})

    find_library(IOKIT IOKit)
    if (NOT IOKIT)
        message(FATAL_ERROR "IOKit not found")
    endif()
    target_link_libraries(palera1n ${IOKIT})
endif()
add_compile_options(palera1n
        -static -no-pie -Wl,--gc-sections
        )


if(!BUILD_DATE)
execute_process(COMMAND date OUTPUT_VARIABLE BUILD_DATE)
endif()
if(!BUILD_TAG)
execute_process(COMMAND git describe --dirty --tags --abbrev=7 OUTPUT_VARIABLE BUILD_TAG)
endif()
if(!BUILD_WHOAMI)
    execute_process(COMMAND whoami OUTPUT_VARIABLE BUILD_WHOAMI)
endif ()

set(INCLUDE_DIR /usr/local/include)
include_directories (${INCLUDE_DIR})


add_compile_definitions(
        BUILD_DATE="${BUILD_DATE}"
        BUILD_TAG="${BUILD_TAG}"
        BUILD_WHOAMI="${BUILD_WHOAMI}"
        BUILD_STYLE="${CMAKE_BUILD_TYPE}"
)
