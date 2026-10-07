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

# M4 視覺樹與雙階段排版引擎測試
add_executable(layout_engine_test layout_engine_test.cpp)
target_link_libraries(layout_engine_test PRIVATE cppui_core)
add_test(NAME LayoutEngineTest COMMAND layout_engine_test)

# M5 輸入分發、焦點管理與路由事件測試
add_executable(input_events_test input_events_test.cpp)
target_link_libraries(input_events_test PRIVATE cppui_core)
add_test(NAME InputEventsTest COMMAND input_events_test)

