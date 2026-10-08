/*
 * File: SkiaDemoForm.hpp
 * Purpose: Declares SkiaDemoForm illustrating forced Google Skia platform backend switching.
 */

#pragma once

#include "ui/app/Form.hpp"

namespace platform_skia_example {

/// <summary>
/// 平台切換 (強制使用 Skia) 示範視窗表單
/// </summary>
class SkiaDemoForm : public cppui::app::Form {
public:
    /// <summary>
    /// 建構 SkiaDemoForm 並初始化介面佈局。
    /// </summary>
    SkiaDemoForm();

    /// <summary>
    /// 解構 SkiaDemoForm 實例。
    /// </summary>
    ~SkiaDemoForm() override = default;

    /// <summary>
    /// 視窗加載生命週期事件回呼。
    /// </summary>
    void OnLoad() override;
};

} // namespace platform_skia_example
