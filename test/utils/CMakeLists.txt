# --------- test_isa_to_string
add_executable(test_isa_to_string EXCLUDE_FROM_ALL
    test_isa_to_string.c)
add_test(test_isa_to_string test_isa_to_string)
target_link_libraries(test_isa_to_string LINK_PUBLIC armvm-utils)
add_dependencies(test_isa_to_string armvm-utils)
add_dependencies(check_memcheck test_isa_to_string)

# --------- test_string_to_isa
add_executable(test_string_to_isa EXCLUDE_FROM_ALL
    test_string_to_isa.c)
add_test(test_string_to_isa test_string_to_isa)
target_link_libraries(test_string_to_isa LINK_PUBLIC armvm-utils)
add_dependencies(test_string_to_isa armvm-utils)
add_dependencies(check_memcheck test_string_to_isa)
