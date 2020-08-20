cmake_minimum_required(VERSION 3.10)


##
## general
#################################################

project(libarmvm)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

include_directories("${PROJECT_SOURCE_DIR}/include"
                    "${PROJECT_BINARY_DIR}")

configure_file(${PROJECT_SOURCE_DIR}/cmake/cli_version.h.in ${PROJECT_BINARY_DIR}/cli_version.h @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/cmake/lib_version.h.in ${PROJECT_BINARY_DIR}/lib_version.h @ONLY)
add_custom_target(version)
add_custom_command(TARGET version
                   PRE_BUILD
                   BYPRODUCTS "${PROJECT_BINARY_DIR}/cli_version.c" "${PROJECT_BINARY_DIR}/lib_version.c"
                   COMMAND ${CMAKE_COMMAND}
                   -Dlocal_dir="${PROJECT_SOURCE_DIR}"
                   -Doutput_dir="${PROJECT_BINARY_DIR}"
                   -P "${PROJECT_SOURCE_DIR}/cmake/gitversion.cmake")


##
## libarmvm-utils
#################################################

add_library(armvm-utils SHARED
    lib/utils/isa_to_string.c
    lib/utils/string_to_isa.c)
target_include_directories(armvm-utils INTERFACE "${PROJECT_SOURCE_DIR}/include")

##
## libarmvm
#################################################

add_library(armvm SHARED
    lib/libarmvm.c
    lib/libarmvm_memory.c
    lib/libarmvm_registers.c
    lib/libarmvm_peripherals.c
    lib/libarmvm_isa.c
    ${PROJECT_BINARY_DIR}/lib_version.c)
target_include_directories(armvm INTERFACE "${PROJECT_SOURCE_DIR}/include"
                                 PRIVATE "${PROJECT_BINARY_DIR}"
                                 PRIVATE "${PROJECT_SOURCE_DIR}/lib")
target_link_libraries(armvm LINK_PUBLIC armvm-utils)
add_dependencies(armvm armvm-utils)


##
## armvm (cli)
#################################################

add_executable(arm-vm
    arm-vm/armvm.c
    arm-vm/armvm_config.c
    ${PROJECT_BINARY_DIR}/cli_version.c)
target_include_directories(arm-vm PRIVATE "${PROJECT_SOURCE_DIR}/arm-vm" 
                                          "${PROJECT_BINARY_DIR}")
target_link_libraries(arm-vm LINK_PUBLIC armvm)
add_dependencies(arm-vm armvm)


##
## testing
#################################################

enable_testing()

# add new target check for unittests
set(CTEST_OUTPUT_ON_FAILURE TRUE)
set(MEMORYCHECK_COMMAND_OPTIONS "-q --tool=memcheck --trace-children=yes --leak-check=yes --error-exitcode=1" CACHE STRING "bla")
include(CTest)

if (NOT TARGET check_memcheck)
    add_custom_target(check_memcheck COMMAND ${CMAKE_CTEST_COMMAND} --forec-new-ctest-process --test-action memcheck -R "^test_.*$")
endif()

if (NOT TARGET check)
    add_custom_target(check DEPENDS check_memcheck)
endif()

add_subdirectory(test)
