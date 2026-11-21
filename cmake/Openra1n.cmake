# =========================
# MARK: NON-CMAKE DEPENDENCIES: OPENRAIN
# =========================

message(STATUS "Building with Openra1n support")

if(IOS)
    execute_process(
        COMMAND xcrun --sdk iphoneos --show-sdk-path
        OUTPUT_VARIABLE IOS_SDK
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E env
            "CFLAGS=-isysroot ${IOS_SDK} -arch arm64 -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -I${CMAKE_SOURCE_DIR}/apple-include-iphoneos"
            "CXXFLAGS=-isysroot ${IOS_SDK} -arch arm64 -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -I${CMAKE_SOURCE_DIR}/apple-include-iphoneos"
            "LDFLAGS=-isysroot ${IOS_SDK} -arch arm64 -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}"
            make CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} AR=${CMAKE_AR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/1stparty/libopenra1n
        RESULT_VARIABLE OPENRA1N_BUILD_RESULT
    )
elseif(APPLE)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E env
            "CFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}"
            make CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/1stparty/libopenra1n
        RESULT_VARIABLE OPENRA1N_BUILD_RESULT
    )
else()
    execute_process(
        COMMAND make CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/1stparty/libopenra1n
        RESULT_VARIABLE OPENRA1N_BUILD_RESULT
    )
endif()

if(NOT OPENRA1N_BUILD_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to build Openra1n.")
endif()

set(OPENRA1N_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/1stparty/libopenra1n/build/include")
set(OPENRA1N_LIB "${CMAKE_SOURCE_DIR}/1stparty/libopenra1n/build/lib/openra1n/libopenra1n.a")

target_include_directories(palera1n PRIVATE ${OPENRA1N_INCLUDE_DIR})
target_link_libraries(palera1n PRIVATE "${OPENRA1N_LIB}")

if(IOS)
    target_include_directories(palera1n PRIVATE "${CMAKE_SOURCE_DIR}/apple-include-iphoneos")
endif()
