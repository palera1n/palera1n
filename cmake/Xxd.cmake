find_program(BASH_EXECUTABLE bash REQUIRED)
find_program(_XXD xxd REQUIRED)
find_program(_JQ jq REQUIRED)
find_program(_XZ xz REQUIRED)

execute_process(
    COMMAND "${BASH_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/xxd.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND_ERROR_IS_FATAL ANY
)
