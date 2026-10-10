cmake_minimum_required(VERSION 3.20)
project(CPPUI VERSION 0.1.0 LANGUAGES CXX C)

# -----------------------------------------------------------------------------
# C++ 標準設定
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# 輸出目錄統一設定 (確保測試與相依 DLL 位於同一目錄，避免 0xc0000135 錯誤)
# -----------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

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

# 針對 MinGW (Windows GCC/Clang) 啟用靜態運行庫鏈結，避免測試執行時缺失 MinGW DLL (0xc0000135)
if(MINGW)
    add_link_options(-static -static-libgcc -static-libstdc++)
endif()

# -----------------------------------------------------------------------------
# AddressSanitizer (ASan) 記憶體安全檢查
# -----------------------------------------------------------------------------
option(CPPUI_ENABLE_ASAN "Enable AddressSanitizer (ASan)" ON)
set(ASAN_DLL "" CACHE INTERNAL "AddressSanitizer runtime DLL")
set(ASAN_DLL_DIR "" CACHE INTERNAL "AddressSanitizer runtime DLL directory")

if(CPPUI_ENABLE_ASAN)
    if(MSVC)
        add_compile_options(/fsanitize=address /Zi)
        add_link_options(/INCREMENTAL:NO /DEBUG)
        get_filename_component(_MSVC_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        find_file(_MSVC_ASAN_DLL
            NAMES clang_rt.asan_dynamic-x86_64.dll
            PATHS "${_MSVC_BIN_DIR}"
            NO_DEFAULT_PATH
        )
        if(_MSVC_ASAN_DLL)
            set(ASAN_DLL "${_MSVC_ASAN_DLL}" CACHE INTERNAL "AddressSanitizer runtime DLL" FORCE)
            set(ASAN_DLL_DIR "${_MSVC_BIN_DIR}" CACHE INTERNAL "AddressSanitizer runtime DLL directory" FORCE)
            message(STATUS "AddressSanitizer runtime DLL detected: ${ASAN_DLL}")
        endif()
    elseif(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
        execute_process(
            COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=clang_rt.asan_dynamic-x86_64.dll
            OUTPUT_VARIABLE _CLANG_ASAN_DLL
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(EXISTS "${_CLANG_ASAN_DLL}")
            set(ASAN_DLL "${_CLANG_ASAN_DLL}" CACHE INTERNAL "AddressSanitizer runtime DLL" FORCE)
            get_filename_component(_CLANG_ASAN_DIR "${_CLANG_ASAN_DLL}" DIRECTORY)
            set(ASAN_DLL_DIR "${_CLANG_ASAN_DIR}" CACHE INTERNAL "AddressSanitizer runtime DLL directory" FORCE)
            message(STATUS "AddressSanitizer runtime DLL detected: ${ASAN_DLL}")
        endif()
    elseif(MINGW)
        message(STATUS "MinGW GCC does not support AddressSanitizer on Windows; skipping ASan for MinGW.")
    else()
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
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
    src/controls/ScrollViewer.cpp
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
    src/app/Timer.cpp
    src/controls/PictureBox.cpp
    src/controls/RadioButton.cpp
    src/controls/ProgressBar.cpp
    src/controls/Slider.cpp
    src/controls/ListBox.cpp
    src/controls/Separator.cpp
    src/controls/Label.cpp
    src/graphics/Pen.cpp
    src/graphics/Pens.cpp
    src/graphics/Brush.cpp
    src/graphics/Brushes.cpp
    src/graphics/GraphicsPath.cpp
    src/graphics/Bitmap.cpp
    src/graphics/Graphics.cpp
    src/graphics/Convolution.cpp
    src/graphics/ImageFilters.cpp
    src/graphics/SkiaImageBridge.cpp
    src/theme/Theme.cpp
    src/theme/ThemeManager.cpp
    src/dialogs/DialogCommon.cpp
    src/dialogs/MessageBox.cpp
    src/dialogs/MessageBoxForm.cpp
    src/dialogs/FileDialog.cpp
    src/dialogs/OpenFileDialog.cpp
    src/dialogs/SaveFileDialog.cpp
    src/dialogs/FileDialogForm.cpp
    src/dialogs/Win32DialogService.cpp
)

if(WIN32)
    set(CPPUI_HAS_X11 0)
    list(APPEND CPPUI_SOURCES
        src/pal/win32/PlatformWindowWin32.cpp
        src/text/DirectWriteTextEngine.cpp
    )
else()
    option(CPPUI_ENABLE_X11 "Enable X11 window backend on Linux" ON)
    set(CPPUI_HAS_X11 0)
    if(CPPUI_ENABLE_X11)
        find_package(X11 QUIET)
        if(X11_FOUND)
            set(CPPUI_HAS_X11 1)
            message(STATUS "X11 detected: ${X11_LIBRARIES}")
            list(APPEND CPPUI_SOURCES src/pal/x11/PlatformWindowX11.cpp)
        endif()
    endif()
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
target_compile_definitions(cppui_core PUBLIC 
    CPPUI_HAS_SKIA=${CPPUI_HAS_SKIA}
    CPPUI_HAS_X11=${CPPUI_HAS_X11}
)

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
        shell32
        comdlg32
    )
else()
    find_package(Threads REQUIRED)
    target_link_libraries(cppui_core PUBLIC Threads::Threads ${CMAKE_DL_LIBS})
    if(CPPUI_HAS_X11)
        target_include_directories(cppui_core PUBLIC ${X11_INCLUDE_DIR})
        target_link_libraries(cppui_core PUBLIC ${X11_LIBRARIES})
    endif()
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
