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
if(MSVC)
    add_compile_options(/W4 /utf-8 /permissive- /Zc:__cplusplus)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# -----------------------------------------------------------------------------
# 核心標頭檔介面 (Interface Library)
# -----------------------------------------------------------------------------
add_library(cppui_headers INTERFACE)
target_include_directories(cppui_headers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/compat>
    $<INSTALL_INTERFACE:include>
)

# -----------------------------------------------------------------------------
# 測試與範例
# -----------------------------------------------------------------------------
option(CPPUI_BUILD_TESTS "Build CPPUI unit and smoke tests" ON)
option(CPPUI_BUILD_EXAMPLES "Build CPPUI example applications" ON)

if(CPPUI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
