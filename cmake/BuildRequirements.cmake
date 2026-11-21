# =========================
# BUILD REQUIREMENTS
# =========================

if(LINUX AND WITH_STATIC)
    if(
        NOT CROSS_HOST_TRIPLE MATCHES "musl"
        AND NOT CMAKE_C_COMPILER_TARGET MATCHES "musl"
        AND NOT CMAKE_C_COMPILER MATCHES "musl"
    )
        message(FATAL_ERROR
            "Static builds on Linux require a musl toolchain. "
            "Please specify a musl cross-compiler or set "
            "-DCROSS_HOST_TRIPLE=...-linux-musl"
        )
    endif()
endif()

if(MINGW AND NOT WITH_STATIC)
    message(FATAL_ERROR
        "MinGW builds must be static. Please configure with -DWITH_STATIC=1"
    )
endif()

if(MINGW AND WITH_TUI)
    message(FATAL_ERROR
        "-DWITH_TUI=1 is not supported on MinGW builds."
    )
endif()

if(MINGW AND WITH_CIDERRAIN)
    message(FATAL_ERROR
        "-DWITH_CIDERRAIN=1 is not supported on MinGW builds."
    )
endif()

if(IOS AND (WITH_TUI OR WITH_GUI))
    message(FATAL_ERROR
        "-DWITH_TUI=1 or -DWITH_GUI=1 is not supported on iOS builds."
    )
endif()
