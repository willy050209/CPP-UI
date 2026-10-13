/**
 * @file Exports.hpp
 * @brief CPPUI 跨平台公開符號匯出與可見性巨集定義 (Symbol Visibility & Shared Library Export)
 */

#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(CPPUI_BUILD_SHARED)
#    define CPPUI_API __declspec(dllexport)
#  elif defined(CPPUI_USE_SHARED)
#    define CPPUI_API __declspec(dllimport)
#  else
#    define CPPUI_API
#  endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#  if defined(CPPUI_BUILD_SHARED)
#    define CPPUI_API __attribute__((visibility("default")))
#  else
#    define CPPUI_API
#  endif
#else
#  define CPPUI_API
#endif
