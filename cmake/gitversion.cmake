cmake_minimum_required(VERSION 3.10)

message(STATUS "Resolving GIT Version")

set(_build_version "unknown")

string(TIMESTAMP _time_stamp)

find_package(Git)
if (GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe
        WORKING_DIRECTORY "${local_dir}"
        OUTPUT_VARIABLE _build_version
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "GIT VERSION: ${_build_version}")
    message(STATUS "GIT BUILD_TIME: ${_time_stamp}")
else()
    message(STATUS "GIT not found")
endif()


configure_file(${local_dir}/cmake/cli_version.c.in ${output_dir}/cli_version.c @ONLY)
configure_file(${local_dir}/cmake/lib_version.c.in ${output_dir}/lib_version.c @ONLY)

