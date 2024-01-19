set(TESTS
    test_tokenizer.cpp
    )

add_catch(test_scheme ${TESTS})
target_link_libraries(test_scheme scheme)
