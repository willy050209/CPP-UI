# -----------------------------------------------------------------------------
# CPPUI 官方獨立視覺化表單設計器 (Standalone Visual Form Designer Tool)
# -----------------------------------------------------------------------------

add_executable(cppui-designer
    main.cpp
    DesignerMainWindow.hpp
    DesignerMainWindow.cpp
)

target_link_libraries(cppui-designer
    PRIVATE
        cppui_core
        cppui_designer_core
)

target_include_directories(cppui-designer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

# 設置二進位檔輸出目錄
set_target_properties(cppui-designer PROPERTIES
    OUTPUT_NAME "cppui-designer"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 安裝導出規則
install(TARGETS cppui-designer
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
