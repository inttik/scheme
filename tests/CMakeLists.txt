set(TESTS
    test_tokenizer.cpp
    test_parser.cpp
    test_basic_functions.cpp
    test_advanced_functions.cpp
    )

add_catch(test_scheme ${TESTS})
target_link_libraries(test_scheme scheme)
