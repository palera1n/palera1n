cmake_minimum_required(VERSION 3.14)
project(palera1n)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CMAKE_CXX_STANDARD 99)
set(SRC ${CMAKE_CURRENT_SOURCE_DIR})
set(DEP ${SRC}/dep_root)
set(CMAKE_C_STANDARD 99)
set(STATIC 0)
set(CMAKE_SUPPRESS_REGENERATION ON)

set(CMAKE_BUILD_TYPE BUILD_TYPE)


add_custom_target(distclean
        COMMAND sh -c "find src -name '*.o' -type f -delete"
        COMMAND sh -c "find src  -perm -u+x -type f -delete"
        )
#add_custom_target(distclean
#        COMMAND sh -c "rm -rf *"
#        )
add_subdirectory(src)
add_subdirectory(docs)
