add_executable(smoke_test smoke_test.cpp)
target_link_libraries(smoke_test PRIVATE cppui_headers)

add_test(NAME SmokeTest COMMAND smoke_test)
