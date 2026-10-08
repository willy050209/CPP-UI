/*
 * File: HelloWorldForm.hpp
 * Purpose: Declares HelloWorldForm displaying a simple greeting and a close button.
 */

#pragma once

#include "ui/app/Form.hpp"

/// <summary>
/// 示範用 Hello World 桌面視窗表單
/// </summary>
class HelloWorldForm : public cppui::app::Form {
public:
    /// <summary>
    /// 建構 HelloWorldForm 並初始化介面佈局。
    /// </summary>
    HelloWorldForm();
    ~HelloWorldForm() override = default;
};
