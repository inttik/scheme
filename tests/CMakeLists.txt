set(TESTS
    test_tokenizer.cpp
    test_parser.cpp
    )

add_catch(test_scheme ${TESTS})
target_link_libraries(test_scheme scheme)
