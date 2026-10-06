# 基礎冒煙測試
add_executable(smoke_test smoke_test.cpp)
target_link_libraries(smoke_test PRIVATE cppui_headers)
add_test(NAME SmokeTest COMMAND smoke_test)

# PAL 與後端單元測試
add_executable(pal_test pal_test.cpp)
target_link_libraries(pal_test PRIVATE cppui_core)
add_test(NAME PalTest COMMAND pal_test)

# M2 DisplayList 與 Arena 測試
add_executable(display_list_test display_list_test.cpp)
target_link_libraries(display_list_test PRIVATE cppui_core)
add_test(NAME DisplayListTest COMMAND display_list_test)

# M3 文字管線與排版測試
add_executable(text_pipeline_test text_pipeline_test.cpp)
target_link_libraries(text_pipeline_test PRIVATE cppui_core)
add_test(NAME TextPipelineTest COMMAND text_pipeline_test)
