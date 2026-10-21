cmake_minimum_required(VERSION 3.20)

# -----------------------------------------------------------------------------
# C++ 執行期函式庫 (CRT) 政策與全域設定 [統一使用動態 CRT MultiThreadedDLL]
# -----------------------------------------------------------------------------
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

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
    add_compile_options(/W4 /utf-8 /permissive- /Zc:__cplusplus /EHsc /FS)
elseif(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # GNU 風格 clang++ 於 Windows 鏈結 MSVC ABI 時，顯式定義 _DLL 避免 MT 標記衝突
    add_compile_definitions(_DLL)
    add_compile_options(-Wall -Wextra -Wpedantic)
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
    if(MSVC AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(/fsanitize=address /Z7)
        add_link_options(/INCREMENTAL:NO /DEBUG)
        get_filename_component(_MSVC_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        find_file(_MSVC_ASAN_DLL
            NAMES clang_rt.asan_dynamic-x86_64.dll clang_rt.asan_dynamic-i386.dll
            PATHS "${_MSVC_BIN_DIR}"
            NO_DEFAULT_PATH
        )
        if(_MSVC_ASAN_DLL)
            set(ASAN_DLL "${_MSVC_ASAN_DLL}" CACHE INTERNAL "AddressSanitizer runtime DLL" FORCE)
            set(ASAN_DLL_DIR "${_MSVC_BIN_DIR}" CACHE INTERNAL "AddressSanitizer runtime DLL directory" FORCE)
            message(STATUS "AddressSanitizer runtime DLL detected: ${ASAN_DLL}")
        endif()
    elseif(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(MSVC)
            # clang-cl 驅動程式搭配 MSVC 相容連結器 (lld-link / link.exe)
            # 編譯端啟用 ASan 插樁，鏈結端僅傳遞 MSVC 合法連結參數 (嚴禁傳入 /fsanitize=address)
            add_compile_options(/fsanitize=address /Z7)
            add_link_options(/INCREMENTAL:NO /DEBUG)

            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                set(_CLANG_ARCH "x86_64")
            else()
                set(_CLANG_ARCH "i386")
            endif()

            execute_process(
                COMMAND ${CMAKE_CXX_COMPILER} /clang:-print-file-name=clang_rt.asan_dynamic-${_CLANG_ARCH}.dll
                OUTPUT_VARIABLE _CLANG_ASAN_DLL
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            execute_process(
                COMMAND ${CMAKE_CXX_COMPILER} /clang:-print-file-name=clang_rt.asan_dynamic-${_CLANG_ARCH}.lib
                OUTPUT_VARIABLE _CLANG_ASAN_LIB
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            execute_process(
                COMMAND ${CMAKE_CXX_COMPILER} /clang:-print-file-name=clang_rt.asan_dynamic_runtime_thunk-${_CLANG_ARCH}.lib
                OUTPUT_VARIABLE _CLANG_ASAN_THUNK_LIB
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )

            if(EXISTS "${_CLANG_ASAN_DLL}")
                set(ASAN_DLL "${_CLANG_ASAN_DLL}" CACHE INTERNAL "AddressSanitizer runtime DLL" FORCE)
                get_filename_component(_CLANG_ASAN_DIR "${_CLANG_ASAN_DLL}" DIRECTORY)
                set(ASAN_DLL_DIR "${_CLANG_ASAN_DIR}" CACHE INTERNAL "AddressSanitizer runtime DLL directory" FORCE)
                message(STATUS "AddressSanitizer runtime DLL detected: ${ASAN_DLL}")
            endif()

            if(EXISTS "${_CLANG_ASAN_LIB}" AND EXISTS "${_CLANG_ASAN_THUNK_LIB}")
                get_filename_component(_CLANG_LIB_DIR "${_CLANG_ASAN_LIB}" DIRECTORY)
                link_directories("${_CLANG_LIB_DIR}")
                link_libraries("clang_rt.asan_dynamic-${_CLANG_ARCH}.lib")
                add_link_options("/WHOLEARCHIVE:clang_rt.asan_dynamic_runtime_thunk-${_CLANG_ARCH}.lib")
                message(STATUS "AddressSanitizer runtime libraries linked: clang_rt.asan_dynamic-${_CLANG_ARCH}.lib, clang_rt.asan_dynamic_runtime_thunk-${_CLANG_ARCH}.lib")
            endif()
        else()
            # GNU 風格 clang++ 驅動程式 (由 clang++ 前端統一調度編譯與鏈結)
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
        endif()
    elseif(MINGW)
        message(STATUS "MinGW GCC does not support AddressSanitizer on Windows; skipping ASan for MinGW.")
    else()
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

# -----------------------------------------------------------------------------
# 相依函式庫：CPP-Compat (向下相容層)
# -----------------------------------------------------------------------------
include(FetchContent)

if(NOT DEFINED FETCHCONTENT_SOURCE_DIR_COMPAT AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../CPP-Compat")
    set(FETCHCONTENT_SOURCE_DIR_COMPAT "${CMAKE_CURRENT_SOURCE_DIR}/../CPP-Compat")
endif()

FetchContent_Declare(
    compat
    GIT_REPOSITORY https://github.com/willy050209/CPP-Compat.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(compat)

# -----------------------------------------------------------------------------
# 核心公共標頭檔介面 (Interface Library)
# -----------------------------------------------------------------------------
include(GNUInstallDirs)

add_library(cppui_headers INTERFACE)
target_include_directories(cppui_headers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(cppui_headers INTERFACE
    $<BUILD_INTERFACE:compat::compat>
    $<INSTALL_INTERFACE:compat::compat>
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
    src/controls/OverlayLayer.cpp
    src/controls/Popup.cpp
    src/controls/ToolTip.cpp
    src/controls/MenuItem.cpp
    src/controls/ContextMenu.cpp
    src/controls/ComboBox.cpp
    src/controls/TreeView.cpp
    src/controls/DataGrid.cpp
    src/layout/VirtualizingStackPanel.cpp
    src/pal/Clipboard.cpp
    src/pal/generic/ClipboardGeneric.cpp
    src/diagnostics/DirtyRectTracker.cpp
    src/diagnostics/VisualTreeInspector.cpp
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
    set(CPPUI_HAS_WAYLAND 0)
    list(APPEND CPPUI_SOURCES
        src/pal/win32/PlatformWindowWin32.cpp
        src/pal/win32/ClipboardWin32.cpp
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
            list(APPEND CPPUI_SOURCES
                src/pal/x11/PlatformWindowX11.cpp
                src/pal/x11/ClipboardX11.cpp
            )
        endif()
    endif()

    option(CPPUI_ENABLE_WAYLAND "Enable Wayland window backend on Linux" ON)
    set(CPPUI_HAS_WAYLAND 0)
    if(CPPUI_ENABLE_WAYLAND)
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(WAYLAND wayland-client wayland-cursor xkbcommon QUIET)
            if(WAYLAND_FOUND)
                set(CPPUI_HAS_WAYLAND 1)
                message(STATUS "Wayland detected: ${WAYLAND_LIBRARIES}")
                list(APPEND CPPUI_SOURCES
                    src/pal/wayland/PlatformWindowWayland.cpp
                    src/pal/wayland/ClipboardWayland.cpp
                    src/pal/wayland/protocol/xdg-shell-protocol.c
                )
            endif()
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
    CPPUI_HAS_WAYLAND=${CPPUI_HAS_WAYLAND}
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
        imm32
    )
else()
    find_package(Threads REQUIRED)
    target_link_libraries(cppui_core PUBLIC Threads::Threads ${CMAKE_DL_LIBS})
    if(CPPUI_HAS_X11)
        target_include_directories(cppui_core PUBLIC ${X11_INCLUDE_DIR})
        target_link_libraries(cppui_core PUBLIC ${X11_LIBRARIES})
    endif()
    if(CPPUI_HAS_WAYLAND)
        target_include_directories(cppui_core PUBLIC ${WAYLAND_INCLUDE_DIRS})
        target_link_libraries(cppui_core PUBLIC ${WAYLAND_LIBRARIES})
    endif()
endif()

# 標準 CMake 別名目標 (供 in-tree / FetchContent 直接引用)
add_library(CppUI::CppUI ALIAS cppui_core)

# -----------------------------------------------------------------------------
# CPPUI Designer 視覺化表單生成器核心領域模型 (cppui_designer_core)
# -----------------------------------------------------------------------------
option(CPPUI_BUILD_DESIGNER "Build CPPUI Visual Form Designer core library" ON)

if(CPPUI_BUILD_DESIGNER)
    set(CPPUI_DESIGNER_SOURCES
        src/designer/ast/WidgetNode.cpp
        src/designer/ast/FormDocumentNode.cpp
        src/designer/ast/CardinalityValidator.cpp
        src/designer/ast/IdentifierValidator.cpp
        src/designer/metadata/PropertyValue.cpp
        src/designer/metadata/PropertyDescriptor.cpp
        src/designer/commands/CommandHistoryManager.cpp
        src/designer/commands/WidgetCommands.cpp
        src/designer/commands/PropertyCommands.cpp
        src/designer/commands/MacroCommand.cpp
        src/designer/serialization/JsonParserInternal.cpp
        src/designer/serialization/FormSerializer.cpp
        src/designer/codegen/CodegenTypes.cpp
        src/designer/codegen/FluentDslEmitter.cpp
        src/designer/codegen/FormClassEmitter.cpp
        src/designer/codegen/CMakeSnippetEmitter.cpp
        src/designer/codegen/FormCodegenPipeline.cpp
        src/designer/surface/SurfaceTypes.cpp
        src/designer/surface/SelectionModel.cpp
        src/designer/surface/DesignerHitTest.cpp
        src/designer/surface/SnapEngine.cpp
        src/designer/surface/AdornerOverlay.cpp
        src/designer/surface/DesignSurface.cpp
        src/designer/inspector/InspectorTypes.cpp
        src/designer/inspector/InheritanceMetadataRegistry.cpp
        src/designer/inspector/PropertyEditors.cpp
        src/designer/inspector/InspectorPanel.cpp
        src/designer/toolbox/ToolboxPanel.cpp
        src/designer/preview/LiveCodePreviewPanel.cpp
        src/designer/preview/CodePreviewWindow.cpp
        src/designer/preview/LiveFormRunner.cpp
        src/designer/exporter/ProjectExporter.cpp
    )

    add_library(cppui_designer_core STATIC ${CPPUI_DESIGNER_SOURCES})
    target_link_libraries(cppui_designer_core PUBLIC cppui_headers)
    target_include_directories(cppui_designer_core
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    add_library(CppUI::DesignerCore ALIAS cppui_designer_core)
endif()

# -----------------------------------------------------------------------------
# 測試與範例 (頂層專案與 FetchContent 子專案隔離)
# -----------------------------------------------------------------------------
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(CPPUI_IS_TOP_LEVEL ON)
else()
    set(CPPUI_IS_TOP_LEVEL OFF)
endif()

option(CPPUI_BUILD_TESTS "Build CPPUI unit and smoke tests" ${CPPUI_IS_TOP_LEVEL})
option(CPPUI_BUILD_EXAMPLES "Build CPPUI example applications" ${CPPUI_IS_TOP_LEVEL})

if(CPPUI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(CPPUI_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

option(CPPUI_BUILD_DESIGNER_TOOL "Build standalone CPPUI Visual Form Designer tool" ${CPPUI_IS_TOP_LEVEL})
if(CPPUI_BUILD_DESIGNER AND CPPUI_BUILD_DESIGNER_TOOL)
    add_subdirectory(tools/designer)
endif()

# -----------------------------------------------------------------------------
# 安裝與導出規則 (Installation and CMake Package Export)
# -----------------------------------------------------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# 安裝目標二進位與標頭檔
set(CPPUI_EXPORT_TARGETS cppui_core cppui_headers compat)
if(CPPUI_BUILD_DESIGNER)
    list(APPEND CPPUI_EXPORT_TARGETS cppui_designer_core)
endif()

install(TARGETS ${CPPUI_EXPORT_TARGETS}
    EXPORT CppUITargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY "${compat_SOURCE_DIR}/include/"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# 導出 Targets 檔案
set(CPPUI_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/CppUI")

install(EXPORT CppUITargets
    FILE CppUITargets.cmake
    NAMESPACE CppUI::
    DESTINATION "${CPPUI_CONFIG_INSTALL_DIR}"
)

# 生成並安裝 CppUIConfig.cmake 與 CppUIConfigVersion.cmake
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CppUIConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CppUIConfig.cmake"
    INSTALL_DESTINATION "${CPPUI_CONFIG_INSTALL_DIR}"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/CppUIConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/CppUIConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/CppUIConfigVersion.cmake"
    DESTINATION "${CPPUI_CONFIG_INSTALL_DIR}"
)

if(CPPUI_HAS_SKIA)
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindSkia.cmake"
        DESTINATION "${CPPUI_CONFIG_INSTALL_DIR}"
    )
endif()

# -----------------------------------------------------------------------------
# CPack 多平台二進位封裝 (Binary Packaging)
# -----------------------------------------------------------------------------
set(CPACK_PACKAGE_NAME "CppUI")
set(CPACK_PACKAGE_VENDOR "CPPUI Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modern C++20 Cross-Platform UI Framework")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "CppUI")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;7Z")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "CPPUI Team")
    set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
endif()

include(CPack)

