# --------- test_type
add_executable(test_type EXCLUDE_FROM_ALL
    test_type.c)
add_test(test_type test_type)
target_link_libraries(test_type LINK_PUBLIC armvm-utils)
add_dependencies(test_type armvm-utils)
add_dependencies(check_memcheck test_type)
