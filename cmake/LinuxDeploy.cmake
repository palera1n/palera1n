# cmake/LinuxDeploy.cmake

include(FetchContent)

set(LINUXDEPLOY_URL
    "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
)

set(LINUXDEPLOY
    "${CMAKE_BINARY_DIR}/linuxdeploy"
)

if(NOT EXISTS "${LINUXDEPLOY}")
    message(STATUS "Downloading linuxdeploy...")

    file(DOWNLOAD
        "${LINUXDEPLOY_URL}"
        "${LINUXDEPLOY}"
        SHOW_PROGRESS
        STATUS LINUXDEPLOY_DOWNLOAD_STATUS
    )

    list(GET LINUXDEPLOY_DOWNLOAD_STATUS 0 LINUXDEPLOY_DOWNLOAD_RESULT)

    if(NOT LINUXDEPLOY_DOWNLOAD_RESULT EQUAL 0)
        list(GET LINUXDEPLOY_DOWNLOAD_STATUS 1 LINUXDEPLOY_DOWNLOAD_ERROR)

        message(FATAL_ERROR
            "Failed to download linuxdeploy: ${LINUXDEPLOY_DOWNLOAD_ERROR}"
        )
    endif()
endif()

file(
    CHMOD "${LINUXDEPLOY}"
    FILE_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
)

set(CPACK_GENERATOR "External")
set(CPACK_EXTERNAL_ENABLE_STAGING YES)

set(CPACK_EXTERNAL_PACKAGE_SCRIPT
    "${CMAKE_CURRENT_BINARY_DIR}/LinuxDeploy-package.cmake"
)

set(LINUXDEPLOY_SCRIPT [=[
set(APPDIR "${CPACK_TEMPORARY_DIRECTORY}")
set(OUTPUT_DIR "@CMAKE_BINARY_DIR@")

set(DESKTOP_FILE
    "${APPDIR}/share/applications/in.palera.palera1n.desktop"
)

message(STATUS "Running linuxdeploy")
message(STATUS "AppDir: ${APPDIR}")
message(STATUS "Output: ${OUTPUT_DIR}")

execute_process(
    COMMAND
        "@LINUXDEPLOY@"
        "--appdir=${APPDIR}"
        "--desktop-file=${DESKTOP_FILE}"
        "--output=appimage"
    WORKING_DIRECTORY "${OUTPUT_DIR}"
    RESULT_VARIABLE RESULT
    OUTPUT_VARIABLE OUTPUT
    ERROR_VARIABLE ERROR
)

if(OUTPUT)
    message(STATUS "${OUTPUT}")
endif()

if(ERROR)
    message(STATUS "${ERROR}")
endif()

if(NOT RESULT EQUAL 0)
    message(FATAL_ERROR
        "linuxdeploy failed with exit code ${RESULT}"
    )
endif()

message(STATUS
    "AppImage generated in ${OUTPUT_DIR}"
)
]=])

string(
    REPLACE
        "@LINUXDEPLOY@"
        "${LINUXDEPLOY}"
        LINUXDEPLOY_SCRIPT
        "${LINUXDEPLOY_SCRIPT}"
)

string(
    REPLACE
        "@CMAKE_BINARY_DIR@"
        "${CMAKE_BINARY_DIR}"
        LINUXDEPLOY_SCRIPT
        "${LINUXDEPLOY_SCRIPT}"
)

file(GENERATE
    OUTPUT "${CPACK_EXTERNAL_PACKAGE_SCRIPT}"
    CONTENT "${LINUXDEPLOY_SCRIPT}"
)
