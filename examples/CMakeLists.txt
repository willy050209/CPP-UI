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
