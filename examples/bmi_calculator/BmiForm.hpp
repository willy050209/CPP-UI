/*
 * File: BmiForm.hpp
 * Purpose: Declares the BmiForm interactive health calculator UI.
 */

#pragma once

#include <memory>
#include "ui/app/Form.hpp"
#include "ui/controls/TextBox.hpp"
#include "ui/controls/Label.hpp"
#include "ui/controls/Button.hpp"

namespace bmi_calculator {

/// <summary>
/// 現代化 BMI 身體質量指數計算機視窗表單
/// </summary>
class BmiForm : public cppui::app::Form {
public:
    /// <summary>
    /// 建構 BmiForm 並初始化介面控制項與佈局。
    /// </summary>
    /// <param name="populateSample">是否預先填入示範數據並執行計算。</param>
    explicit BmiForm(bool populateSample = false);

    ~BmiForm() override = default;

private:
    /// <summary>執行 BMI 計算並更新介面顯示。</summary>
    void CalculateBmi();

    /// <summary>重設所有輸入方塊與結果標籤。</summary>
    void ResetFields();

    std::shared_ptr<cppui::controls::TextBox> m_heightBox;
    std::shared_ptr<cppui::controls::TextBox> m_weightBox;
    std::shared_ptr<cppui::controls::Label>   m_resultValueLabel;
    std::shared_ptr<cppui::controls::Label>   m_resultCategoryLabel;
    std::shared_ptr<cppui::controls::Label>   m_adviceLabel;
    std::shared_ptr<cppui::controls::Button>  m_calcButton;
    std::shared_ptr<cppui::controls::Button>  m_resetButton;
};

} // namespace bmi_calculator
