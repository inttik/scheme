include(sources.cmake)

target_include_directories(scheme PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable(scheme_repl repl/main.cpp)
target_link_libraries(scheme_repl scheme)
