add_executable(m1_window_demo m1_window_demo/main.cpp)
target_link_libraries(m1_window_demo PRIVATE cppui_core)

add_executable(m2_displaylist_demo m2_displaylist_demo/main.cpp)
target_link_libraries(m2_displaylist_demo PRIVATE cppui_core)

add_executable(m3_text_demo m3_text_demo/main.cpp)
target_link_libraries(m3_text_demo PRIVATE cppui_core)

add_executable(m4_layout_demo m4_layout_demo/main.cpp)
target_link_libraries(m4_layout_demo PRIVATE cppui_core)
