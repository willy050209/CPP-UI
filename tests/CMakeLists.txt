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

# 坐標轉換管理類別測試
add_executable(coordinate_transform_test coordinate_transform_test.cpp)
target_link_libraries(coordinate_transform_test PRIVATE cppui_headers)
add_test(NAME CoordinateTransformTest COMMAND coordinate_transform_test)

# M6 核心控制項與網格排版測試
add_executable(controls_test controls_test.cpp)
target_link_libraries(controls_test PRIVATE cppui_core)
add_test(NAME ControlsTest COMMAND controls_test)

# M6 宣告式 Fluent RAD DSL 與響應式信號測試
add_executable(fluent_dsl_test fluent_dsl_test.cpp)
target_link_libraries(fluent_dsl_test PRIVATE cppui_core)
add_test(NAME FluentDslTest COMMAND fluent_dsl_test)

# 跨平台通用 PAL 視窗測試 (無顯卡/Headless/事件注入/離線 Framebuffer)
add_executable(pal_crossplatform_test pal_crossplatform_test.cpp)
target_link_libraries(pal_crossplatform_test PRIVATE cppui_core)
add_test(NAME PalCrossPlatformTest COMMAND pal_crossplatform_test)

# Skia 向量繪圖核心與雙引擎測試
add_executable(skia_backend_test skia_backend_test.cpp)
target_link_libraries(skia_backend_test PRIVATE cppui_core)
add_test(NAME SkiaBackendTest COMMAND skia_backend_test)

# M7 依賴反轉 (IoC) 容器測試
add_executable(ioc_container_test ioc_container_test.cpp)
target_link_libraries(ioc_container_test PRIVATE cppui_core)
add_test(NAME IoCContainerTest COMMAND ioc_container_test)

# M7 現代 MVVM 架構測試
add_executable(mvvm_test mvvm_test.cpp)
target_link_libraries(mvvm_test PRIVATE cppui_core)
add_test(NAME MvvmTest COMMAND mvvm_test)

# M7 Application 與 Form 視窗生命週期測試
add_executable(app_form_test app_form_test.cpp)
target_link_libraries(app_form_test PRIVATE cppui_core)
add_test(NAME AppFormTest COMMAND app_form_test)

# M8 GDI+ 圖形 API、Skia 加速、執行緒安全自動同步與 PictureBox 測試
add_executable(graphics_gdi_test graphics_gdi_test.cpp)
target_link_libraries(graphics_gdi_test PRIVATE cppui_core)
add_test(NAME GraphicsGdiTest COMMAND graphics_gdi_test)

# 範例功能全自動測試 (平台切換、非同步事件、心臟線與漸層繪圖)
add_executable(example_features_test example_features_test.cpp)
target_link_libraries(example_features_test PRIVATE cppui_core)
add_test(NAME ExampleFeaturesTest COMMAND example_features_test)




