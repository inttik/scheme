set(TESTS
    empty_test.cpp)

add_catch(test_scheme ${TESTS})
target_link_libraries(test_scheme scheme)
