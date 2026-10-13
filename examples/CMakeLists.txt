add_executable(m1_window_demo m1_window_demo/main.cpp)
target_link_libraries(m1_window_demo PRIVATE cppui_core)

add_executable(m2_displaylist_demo m2_displaylist_demo/main.cpp)
target_link_libraries(m2_displaylist_demo PRIVATE cppui_core)

add_executable(m3_text_demo m3_text_demo/main.cpp)
target_link_libraries(m3_text_demo PRIVATE cppui_core)

add_executable(m4_layout_demo m4_layout_demo/main.cpp)
target_link_libraries(m4_layout_demo PRIVATE cppui_core)

add_executable(m5_input_demo m5_input_demo/main.cpp)
target_link_libraries(m5_input_demo PRIVATE cppui_core)

add_executable(m6_fluent_rad_demo m6_fluent_rad_demo/main.cpp)
target_link_libraries(m6_fluent_rad_demo PRIVATE cppui_core)

add_executable(m7_mvvm_app_demo m7_mvvm_app_demo/main.cpp)
target_link_libraries(m7_mvvm_app_demo PRIVATE cppui_core)

# 1. Hello World Modular Example
add_executable(hello_world
    hello_world/main.cpp
    hello_world/HelloWorldForm.cpp
)
target_link_libraries(hello_world PRIVATE cppui_core)

# 2. MVVM Modular Example
add_executable(mvvm_example
    mvvm_example/main.cpp
    mvvm_example/models/CounterModel.cpp
    mvvm_example/viewmodels/CounterViewModel.cpp
    mvvm_example/views/CounterForm.cpp
)
target_link_libraries(mvvm_example PRIVATE cppui_core)

# 3. IoC Modular Example
add_executable(ioc_example
    ioc_example/main.cpp
    ioc_example/services/ConsoleLogger.cpp
    ioc_example/services/OrderService.cpp
    ioc_example/viewmodels/OrderViewModel.cpp
    ioc_example/views/OrderForm.cpp
)
target_link_libraries(ioc_example PRIVATE cppui_core)

# 4. Platform Backend Switching Example (Forced Skia)
add_executable(platform_skia_example
    platform_skia_example/main.cpp
    platform_skia_example/SkiaDemoForm.cpp
)
target_link_libraries(platform_skia_example PRIVATE cppui_core)

# 5. Asynchronous Events & Dispatcher Example
add_executable(async_events_example
    async_events_example/main.cpp
    async_events_example/AsyncEventForm.cpp
)
target_link_libraries(async_events_example PRIVATE cppui_core)

# 6. GDI+ Graphics Drawing Example (Cardioid & Gradient)
add_executable(drawing_cardioid_example
    drawing_cardioid_example/main.cpp
    drawing_cardioid_example/CardioidForm.cpp
)
target_link_libraries(drawing_cardioid_example PRIVATE cppui_core)

# 7. 對話框展示庫範例 (MessageBox, OpenFileDialog, SaveFileDialog)
add_executable(dialog_gallery
    dialog_gallery/main.cpp
)
target_link_libraries(dialog_gallery PRIVATE cppui_core)

# 8. BMI 計算機教學範例
add_executable(bmi_calculator
    bmi_calculator/main.cpp
    bmi_calculator/BmiForm.cpp
)
target_link_libraries(bmi_calculator PRIVATE cppui_core)

# 9. 圖形函數、圖片繪製與 Grayscale 灰階濾鏡展示範例
add_executable(graphics_drawing_demo
    graphics_drawing_demo/main.cpp
    graphics_drawing_demo/DrawingDemoForm.cpp
)
target_link_libraries(graphics_drawing_demo PRIVATE cppui_core)

# 10. 彈出層與浮動控制項展示庫 (Popup, ToolTip, ContextMenu, ComboBox)
add_executable(popup_gallery
    popup_gallery/main.cpp
)
target_link_libraries(popup_gallery PRIVATE cppui_core)


