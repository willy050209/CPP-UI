cmake_minimum_required(VERSION 3.20)
project(CPPUI VERSION 0.1.0 LANGUAGES CXX C)

# -----------------------------------------------------------------------------
# C++ 標準設定
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# 編譯器與平台旗標
# -----------------------------------------------------------------------------
if(WIN32)
    add_compile_definitions(UNICODE _UNICODE)
endif()

if(MSVC)
    add_compile_options(/W4 /utf-8 /permissive- /Zc:__cplusplus)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# -----------------------------------------------------------------------------
# 核心公共標頭檔介面 (Interface Library)
# -----------------------------------------------------------------------------
add_library(cppui_headers INTERFACE)
target_include_directories(cppui_headers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/compat>
    $<INSTALL_INTERFACE:include>
)

# -----------------------------------------------------------------------------
# CPPUI 核心與 PAL 實作庫 (cppui_pal)
# -----------------------------------------------------------------------------
set(CPPUI_SOURCES
    src/core/Visual.cpp
    src/core/UIElement.cpp
    src/layout/Panel.cpp
    src/layout/StackPanel.cpp
    src/layout/Border.cpp
    src/layout/Canvas.cpp
    src/layout/Grid.cpp
    src/layout/LayoutManager.cpp
    src/controls/TextBlock.cpp
    src/controls/Control.cpp
    src/controls/ContentControl.cpp
    src/controls/Button.cpp
    src/controls/CheckBox.cpp
    src/controls/TextBox.cpp
    src/rendering/DisplayListRenderer.cpp
    src/rendering/backend/SoftwareRenderContext.cpp
    src/rendering/backend/SkiaRenderContext.cpp
    src/text/FontManager.cpp
    src/text/TextShaper.cpp
    src/text/BiDiAnalyzer.cpp
    src/text/LineBreaker.cpp
    src/text/TextLayout.cpp
    src/text/TextLayoutEngine.cpp
    src/pal/PlatformWindowFactory.cpp
    src/pal/generic/PlatformWindowGeneric.cpp
    src/input/RoutedEvent.cpp
    src/input/EventRoute.cpp
    src/input/FocusManager.cpp
    src/input/InputManager.cpp
    src/ioc/ServiceCollection.cpp
    src/mvvm/ViewModelBase.cpp
    src/mvvm/RelayCommand.cpp
    src/app/Form.cpp
    src/app/Application.cpp
    src/app/AppBuilder.cpp
    src/app/Dispatcher.cpp
)

if(WIN32)
    list(APPEND CPPUI_SOURCES
        src/pal/win32/PlatformWindowWin32.cpp
        src/text/DirectWriteTextEngine.cpp
    )
endif()

# -----------------------------------------------------------------------------
# Skia 向量繪圖核心偵測與設定
# -----------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
option(CPPUI_ENABLE_SKIA "Enable Google Skia 2D rendering engine" ON)
set(CPPUI_HAS_SKIA 0)

if(CPPUI_ENABLE_SKIA)
    find_package(Skia QUIET)
    if(Skia_FOUND)
        set(CPPUI_HAS_SKIA 1)
        message(STATUS "Google Skia detected: ${SKIA_LIBRARIES}")
    else()
        message(STATUS "Google Skia not found. Using SoftwareRenderTarget fallback.")
    endif()
endif()

add_library(cppui_core STATIC ${CPPUI_SOURCES})
target_link_libraries(cppui_core PUBLIC cppui_headers)
target_compile_definitions(cppui_core PUBLIC CPPUI_HAS_SKIA=${CPPUI_HAS_SKIA})

if(Skia_FOUND)
    target_link_libraries(cppui_core PUBLIC Skia::Skia)
endif()

if(WIN32)
    target_link_libraries(cppui_core PRIVATE
        user32
        gdi32
        dwmapi
        shcore
        dwrite
        ole32
    )
else()
    find_package(Threads REQUIRED)
    target_link_libraries(cppui_core PUBLIC Threads::Threads)
endif()

# -----------------------------------------------------------------------------
# 測試與範例
# -----------------------------------------------------------------------------
option(CPPUI_BUILD_TESTS "Build CPPUI unit and smoke tests" ON)
option(CPPUI_BUILD_EXAMPLES "Build CPPUI example applications" ON)

if(CPPUI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(CPPUI_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
