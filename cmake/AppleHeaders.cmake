find_program(BASH_EXECUTABLE bash REQUIRED)
find_program(_GSED gsed REQUIRED)

execute_process(
    COMMAND "${BASH_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/apple-headers.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND_ERROR_IS_FATAL ANY
)
