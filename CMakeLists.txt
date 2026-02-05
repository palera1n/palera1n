# Project
cmake_minimum_required(VERSION 3.10)
project(palera1n VERSION 3.0)
include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -DWX_PRECOMP")

# Option for GUI (wxWidgets)
option(WITH_GUI "Build with wxWidgets GUI" OFF)
option(WITH_STATIC "Build with static wxWidgets" OFF)

# Architectures
set(CMAKE_OSX_ARCHITECTURES "arm64")
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13")

# Deps
if(WITH_GUI)
if(WITH_STATIC)
    set(wxBUILD_SHARED OFF)
    FetchContent_Declare(
        wxWidgets
        GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
        GIT_SHALLOW ON
    )
    FetchContent_MakeAvailable(wxWidgets)
    set(WX_LIBRARIES wxWidgets::core wxWidgets::base wxWidgets::net)
else()
    find_package(wxWidgets REQUIRED COMPONENTS core base net)
    include(${wxWidgets_USE_FILE})
    set(WX_LIBRARIES ${wxWidgets_LIBRARIES})
endif()
endif()

# Files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.c" "src/*.h" "src/*.hpp")

if(APPLE AND WITH_GUI)
    add_executable(palera1n MACOSX_BUNDLE ${SOURCES})
    set_target_properties(palera1n PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist)
elseif(APPLE)
    add_executable(palera1n ${SOURCES})
else()
    add_executable(palera1n WIN32 ${SOURCES} resources/main.exe.manifest)
endif()

if(APPLE)
    find_library(IOKIT IOKit)
    find_library(COREFOUNDATION CoreFoundation)
    target_link_libraries(palera1n PRIVATE ${IOKIT} ${COREFOUNDATION})
endif()

if(WITH_GUI)
if(WITH_STATIC)
    target_link_libraries(palera1n PRIVATE wxcore wxnet)
else()
    target_link_libraries(palera1n PRIVATE ${wxWidgets_LIBRARIES})
endif()
target_compile_definitions(palera1n PRIVATE WITH_GUI)
endif()
