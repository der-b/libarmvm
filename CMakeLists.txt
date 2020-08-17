cmake_minimum_required(VERSION 3.10)
project(libarmvm)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

include_directories("${PROJECT_SOURCE_DIR}/include")

enable_testing()

# add new target check for unittests
set(CTEST_OUTPUT_ON_FAILURE TRUE)
set(MEMORYCHECK_COMMAND_OPTIONS "-q --tool=memcheck --trace-children=yes --leak-check=yes --error-exitcode=1" CACHE STRING "bla")
include(CTest)

if (NOT TARGET check_memcheck)
    add_custom_target(check_memcheck COMMAND ${CMAKE_CTEST_COMMAND} --forec-new-ctest-process --test-action memcheck -R "^test_.*$")
endif()

if (NOT TARGET check_threads)
    add_custom_target(check_threads COMMAND ${CMAKE_CTEST_COMMAND} --forec-new-ctest-process --test-action memcheck --overwrite MemoryCheckCommandOptions='--tool=helgrind --error-exitcode=1 -q' -R "^threads_.*$")
endif()

if (NOT TARGET check)
    add_custom_target(check DEPENDS check_memcheck check_threads)
endif()

#add_subdirectory(test)

##
## libarmvm
##
add_library(armvm SHARED
    lib/libarmvm.c)
target_include_directories(armvm INTERFACE "${PROJECT_SOURCE_DIR}/include")

##
## armvm (cli)
##
add_executable(arm-vm
    arm-vm/armvm.c
    arm-vm/armvm_config.c)
target_include_directories(arm-vm PRIVATE "${PROJECT_SOURCE_DIR}/arm-vm")
target_link_libraries(arm-vm LINK_PUBLIC armvm)
add_dependencies(arm-vm armvm)
