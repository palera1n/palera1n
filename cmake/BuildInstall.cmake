# =========================
# MARK: INSTALL
# =========================

if(LINUX AND NOT MINGW)
    install(TARGETS palera1n RUNTIME DESTINATION bin)
elseif(IOS)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/Debug-iphoneos/palera1n.app/palera1n" DESTINATION bin)
elseif(APPLE AND WITH_GUI)
    install(TARGETS palera1n BUNDLE DESTINATION .)
elseif(APPLE)
    install(TARGETS palera1n RUNTIME DESTINATION bin)
elseif(MINGW)
    install(TARGETS palera1n
        RUNTIME DESTINATION .
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/libstdc++-6.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/libgcc_s_seh-1.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/libwinpthread-1.dll"
        DESTINATION .
    )

    if(NOT WITH_CIDERRAIN)
        install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/libusb-1.0.dll"
            DESTINATION .
        )
    endif()
endif()

if((LINUX AND NOT MINGW) OR IOS OR (APPLE AND NOT IOS AND NOT WITH_GUI))
    install(FILES "${CMAKE_SOURCE_DIR}/docs/palera1n.1" DESTINATION share/man/man1)
    install(FILES "${CMAKE_SOURCE_DIR}/docs/p1ctl.8" DESTINATION share/man/man8)
endif()

set(CPACK_PACKAGE_NAME "palera1n")
set(CPACK_PACKAGE_VERSION "0")
set(CPACK_PACKAGE_FILE_NAME "palera1n")

if(MINGW)
    set(CPACK_GENERATOR "NSIS64")

    set(CPACK_PACKAGE_INSTALL_DIRECTORY "palera1n")
    set(CPACK_NSIS_DISPLAY_NAME "palera1n")
    set(CPACK_NSIS_PACKAGE_NAME "palera1n")
    set(CPACK_NSIS_BRANDING_TEXT "palera1n Setup")

    set(CPACK_NSIS_MODIFY_PATH OFF)
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/resources/palera1n.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/resources/palera1n.ico")

    set(CPACK_PACKAGE_EXECUTABLES "palera1n" "palera1n")

    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut \\\"\\$DESKTOP\\\\palera1n.lnk\\\" \\\"\\$INSTDIR\\\\bin\\\\palera1n.exe\\\""
    )

    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete \\\"\\$DESKTOP\\\\palera1n.lnk\\\""
    )
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
elseif(APPLE AND WITH_GUI)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_PACKAGING_INSTALL_PREFIX "/")
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
