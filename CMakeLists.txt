#####################################################################
# palera1n-c cmake file
#####################################################################
cmake_minimum_required(VERSION 3.13)
project(palera1n-gui)
set(SRC "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${SRC}/output")
execute_process(COMMAND bash -c "uname -m | tr -d '\n'" OUTPUT_VARIABLE ARCH)
string(ASCII 27 Esc)
set(reset "${Esc}[m")
set(BoldRed     "${Esc}[1;31m")
set(BoldGreen   "${Esc}[1;32m")
set(BoldBlue    "${Esc}[1;34m")


#####################################################################
# Setting up git and build information
#####################################################################
message("${BoldBlue}==>${reset} Configuring palera1n")
execute_process(COMMAND bash -c "echo RELEASE | tr -d '\n'"  OUTPUT_VARIABLE BUILD_STYLE)
execute_process(COMMAND bash -c "LANG=C date | tr -d '\n'" OUTPUT_VARIABLE BUILD_DATE)
execute_process(COMMAND bash -c "echo staturnz | tr -d '\n'" OUTPUT_VARIABLE BUILD_WHOAMI)

if(EXISTS "${SRC}/.github")
    execute_process(COMMAND bash -c "git describe --dirty --tags --abbrev=7 | tr -d '\n'"  OUTPUT_VARIABLE BUILD_TAG)
    execute_process(COMMAND bash -c "git rev-list --count HEAD | tr -d '\n'" OUTPUT_VARIABLE BUILD_NUMBER)
    execute_process(COMMAND bash -c "git rev-parse --abbrev-ref HEAD | tr -d '\n'" OUTPUT_VARIABLE BUILD_BRANCH)
    execute_process(COMMAND bash -c "git rev-parse HEAD | tr -d '\n'" OUTPUT_VARIABLE BUILD_COMMIT)
else()
    set(BUILD_TAG "UNKNOWN")
    set(BUILD_NUMBER "UNKNOWN")
    set(BUILD_BRANCH "UNKNOWN")
    set(BUILD_COMMIT "UNKNOWN")
endif() 

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBUILD_STYLE='\"${BUILD_STYLE}\"' -DBUILD_DATE='\"${BUILD_DATE}\"' ")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBUILD_WHOAMI='\"${BUILD_WHOAMI}\"' -DBUILD_TAG='\"${BUILD_TAG}\"' ")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBUILD_NUMBER='\"${BUILD_NUMBER}\"' -DBUILD_BRANCH='\"${BUILD_BRANCH}\"' ")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBUILD_COMMIT='\"${BUILD_COMMIT}\"' -DBUILD_WITH='\"cmake version ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}\"' ")
message("${BoldBlue}==>${reset} Build Information")
list(APPEND CMAKE_MESSAGE_INDENT "    - ")
message("GitHub Tag: ${BoldGreen}${BUILD_TAG}${reset}")
message("GitHub Build Number: ${BoldGreen}${BUILD_NUMBER}${reset}")
message("GitHub Branch: ${BoldGreen}${BUILD_BRANCH}${reset}")
message("GitHub Commit: ${BoldGreen}${BUILD_COMMIT}${reset}")
message("Build Style: ${BoldGreen}${BUILD_STYLE}${reset}")
message("Build Date: ${BoldGreen}${BUILD_DATE}${reset}")
message("Built By: ${BoldGreen}${BUILD_WHOAMI}${reset}")
message("Cmake Version: ${BoldGreen}${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}${reset}")
list(POP_BACK CMAKE_MESSAGE_INDENT)


#####################################################################
# Downloading checkra1n, kpf, ramdisk, & bin pack 
#####################################################################
message("${BoldBlue}==>${reset} Downloading Files")
function(download_file url filename)
    if(EXISTS ${SRC}/src/${filename})
    else()
        execute_process(COMMAND ${CMAKE_COMMAND} -E echo_append "${BoldBlue}==>${reset} Downloading ${filename}")
        file(DOWNLOAD "${url}" "${SRC}/src/${filename}")
        if(EXISTS ${SRC}/src/${filename})
            message("${BoldGreen} - [done]${reset}")
        else()
            message("${BoldRed} - [failed]${reset}")
        endif()
    endif()
endfunction()

download_file("https://cdn.nickchan.lol/palera1n/artifacts/kpf/checkra1n-kpf-pongo" "checkra1n-kpf-pongo")
download_file("https://cdn.nickchan.lol/palera1n/artifacts/jbinit_rootless/ramdisk.dmg" "ramdisk.dmg")
download_file("https://cdn.nickchan.lol/palera1n/c-rewrite/deps/binpack.dmg" "binpack.dmg")
download_file("https://assets.checkra.in/downloads/preview/0.1337.1/checkra1n-macos" "checkra1n-macos")
download_file("https://assets.checkra.in/downloads/preview/0.1337.1/checkra1n-linux-arm64" "checkra1n-linux-arm64")
download_file("https://assets.checkra.in/downloads/preview/0.1337.1/checkra1n-linux-arrmel" "checkra1n-linux-armel")
download_file("https://assets.checkra.in/downloads/preview/0.1337.1/checkra1n-linux-x86" "checkra1n-linux-x86")
download_file("https://assets.checkra.in/downloads/preview/0.1337.1/checkra1n-linux-x86_64" "checkra1n-linux-x86_64")

if(APPLE) 
    message("${BoldBlue}==>${reset} Making thin binary for ${ARCH} macos")
    execute_process(COMMAND bash -c "cd ${SRC}/src && lipo -thin ${ARCH} -output checkra1n checkra1n-macos")
endif(APPLE)
unset(APPLE CACHE)

if(LINUX)
    execute_process(COMMAND bash -c "cd ${SRC}/src && mv checkra1n-linux-${ARCH} checkra1n")
endif(LINUX)
unset(LINUX CACHE)


#####################################################################
# Setting our source files
#####################################################################
message("${BoldBlue}==>${reset} Setting source files")
set(SOURCES
    ${SRC}/src/main.c ${SRC}/src/dfuhelper.c
    ${SRC}/src/devhelper.c ${SRC}/src/lockdown_helper.c ${SRC}/src/optparse.c
    ${SRC}/src/override_file.c ${SRC}/src/log.c ${SRC}/src/lock_vars.c
    ${SRC}/src/credits.c ${SRC}/src/fake_embedded.c ${SRC}/src/exec_checkra1n.c
    ${SRC}/src/pongo_helper.c ${SRC}/src/boyermoore_memmem.c ${SRC}/src/usb_iokit.c
    ${SRC}/src/xxd-ramdisk.dmg.c ${SRC}/src/xxd-checkra1n-kpf-pongo.c
    ${SRC}/src/xxd-binpack.dmg.c ${SRC}/src/xxd-checkra1n.c
)
add_executable(palera1n ${SOURCES})
add_custom_command(TARGET palera1n POST_BUILD COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --green "Palera1n build done!")


#####################################################################
# Setting our CFLAGS, LIBS, LDFLAGS, etc
#####################################################################
message("${BoldBlue}==>${reset} Configuring flags")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${SRC}/include -I${SRC} -DCLI_BUILD=1")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -DPALERAIN_VERSION='\"2.0.0\"' -Wall -Wextra -Wno-unused-parameter")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable -I${SRC}/src -std=c99 -pedantic-errors -D_C99_SOURCE -D_POSIX_C_SOURCE=200112L -D_DARWIN_C_SOURCE")

execute_process(COMMAND bash -c "cd ${SRC}/src && xxd -iC ramdisk.dmg > xxd-ramdisk.dmg.c")
execute_process(COMMAND bash -c "cd ${SRC}/src && xxd -iC checkra1n-kpf-pongo > xxd-checkra1n-kpf-pongo.c")
execute_process(COMMAND bash -c "cd ${SRC}/src && xxd -iC binpack.dmg > xxd-binpack.dmg.c")
execute_process(COMMAND bash -c "cd ${SRC}/src && xxd -iC checkra1n > xxd-checkra1n.c")


#####################################################################
# Linking with Libraries and settings some more flags
#####################################################################
message("${BoldBlue}==>${reset} Linking libraries")
if(APPLE) 
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-nullability-extension")
    target_link_libraries(palera1n PUBLIC "-framework CoreFoundation")
    target_link_libraries(palera1n PUBLIC "-framework IOKit")
    set_target_properties(palera1n PROPERTIES LINK_FLAGS "-Wl,-F/Library/Frameworks")
    add_compile_options("-Wl,-dead_strip")
    link_libraries("-Wl,-dead_strip")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdata-sections -ffunction-sections")
    add_compile_options("-static -no-pie -Wl,--gc-sections")
    link_libraries("-static -no-pie -Wl,--gc-sections")
endif(APPLE)
unset(APPLE CACHE)

message("${BoldBlue}==>${reset} Checking pkg-config")
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBIMOBILEDEVICE REQUIRED IMPORTED_TARGET libimobiledevice-1.0)
pkg_check_modules(LIBIMOBILEDEVICEGLUE REQUIRED IMPORTED_TARGET libimobiledevice-glue-1.0)
pkg_check_modules(LIBIRECOVERY REQUIRED IMPORTED_TARGET libirecovery-1.0)
pkg_check_modules(LIBUSBMUXD REQUIRED IMPORTED_TARGET libusbmuxd-2.0)
pkg_check_modules(LIBPLIST REQUIRED IMPORTED_TARGET libplist++-2.0)
#pkg_check_modules(LIBREADLINE REQUIRED IMPORTED_TARGET readline)

target_link_libraries(palera1n PUBLIC PkgConfig::LIBIMOBILEDEVICE)
target_link_libraries(palera1n PUBLIC PkgConfig::LIBIRECOVERY)
target_link_libraries(palera1n PUBLIC PkgConfig::LIBUSBMUXD)
target_link_libraries(palera1n PUBLIC PkgConfig::LIBIMOBILEDEVICEGLUE)
target_link_libraries(palera1n PUBLIC PkgConfig::LIBPLIST)
#target_link_libraries(palera1n PUBLIC PkgConfig::LIBREADLINE)

# temp way for mac
if(${ARCH} MATCHES "x86_64") 
    set(MbedTLS_DIR /usr/local/Cellar/mbedtls/3.3.0/cmake/)
    find_package(MbedTLS REQUIRED)
    target_link_libraries(palera1n PUBLIC MbedTLS::mbedtls)
else()
    set(MbedTLS_DIR /opt/homebrew/Cellar/mbedtls/3.3.0/cmake/)
    find_package(MbedTLS REQUIRED)
    target_link_libraries(palera1n PUBLIC MbedTLS::mbedtls)
endif()

message("${BoldBlue}==>${reset} ${BoldGreen}Done configuring!${reset}")
message("${BoldBlue}==>${reset} ${BoldGreen}You can now run 'make' to build palera1n${reset}")