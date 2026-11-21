# =========================
# MARK: NON-CMAKE DEPENDENCIES: CIDERRAIN
# =========================

message(STATUS "Building with Ciderra1n support")

# TODO: get rid
execute_process(
    COMMAND patch --batch --forward -p1 -i ${CMAKE_SOURCE_DIR}/patches/liteusb/0001-cpp-compatible.patch
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/1stparty/libciderra1n/external/liteusb
    RESULT_VARIABLE LITEUSB_PATCH_RESULT
)

if(IOS)
    execute_process(
        COMMAND xcrun --sdk iphoneos --show-sdk-path
        OUTPUT_VARIABLE IOS_SDK
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E env
            "EXTRA_CFLAGS=-isysroot ${IOS_SDK} -arch arm64 -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -I${CMAKE_SOURCE_DIR}/apple-include-iphoneos"
            "CXXFLAGS=-isysroot ${IOS_SDK} -arch arm64 -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -I${CMAKE_SOURCE_DIR}/apple-include-iphoneos"
            "LDFLAGS=-isysroot ${IOS_SDK} -arch arm64 -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}"
            make CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} AR=${CMAKE_AR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/1stparty/libciderra1n
        RESULT_VARIABLE CIDERRA1N_BUILD_RESULT
    )
elseif(APPLE)
    execute_process(
        COMMAND make CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} EXTRA_CFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/1stparty/libciderra1n
        RESULT_VARIABLE CIDERRA1N_BUILD_RESULT
    )
else()
    execute_process(
        COMMAND make CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/1stparty/libciderra1n
        RESULT_VARIABLE CIDERRA1N_BUILD_RESULT
    )
endif()

if(NOT CIDERRA1N_BUILD_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to build Ciderra1n.")
endif()

set(CIDERRAIN_INCLUDE_DIR   "${CMAKE_SOURCE_DIR}/1stparty/libciderra1n/build/include")
set(CIDERRAIN_LIB           "${CMAKE_SOURCE_DIR}/1stparty/libciderra1n/build/lib/ciderra1n/libciderra1n.a")
target_include_directories(palera1n PRIVATE ${CIDERRAIN_INCLUDE_DIR})
target_link_libraries(palera1n PRIVATE "${CIDERRAIN_LIB}")

if(IOS)
    target_include_directories(palera1n PRIVATE "${CMAKE_SOURCE_DIR}/apple-include-iphoneos")
endif()

if(LINUX)
    set(LITEUSB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/1stparty/libciderra1n/external/liteusb/include")
    target_compile_definitions(palera1n PRIVATE LITEUSB_PLATFORM_LINUX)
    target_include_directories(palera1n PRIVATE ${LITEUSB_INCLUDE_DIR})
endif()
