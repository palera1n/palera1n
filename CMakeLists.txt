cmake_minimum_required(VERSION 3.14)
project(palera1n)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

set(CMAKE_CXX_STANDARD 99)

set(SRC ${CMAKE_CURRENT_SOURCE_DIR})
set(DEP ${SRC}/dep_root)

set(CMAKE_C_STANDARD 99)


set(STATIC 0)






add_subdirectory(src)
add_subdirectory(docs)
