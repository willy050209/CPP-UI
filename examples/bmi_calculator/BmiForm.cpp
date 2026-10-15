/*
 * File: BmiForm.cpp
 * Purpose: Implements the BmiForm interactive calculator UI and BMI calculation logic.
 */

#include "BmiForm.hpp"
#include "ui/dsl/Fluent.hpp"
#include "compat/Compat.hpp"

namespace bmi_calculator {

/// <summary>
/// 建構 BmiForm 並初始化介面控制項與佈局。
/// </summary>
BmiForm::BmiForm(bool populateSample) {
    using namespace cppui::dsl;
    using namespace cppui::core;

    SetTitle("BMI 健康計算機 - CPPUI");
    SetFormSize(420, 520);

    m_heightBox = std::make_shared<cppui::controls::TextBox>("例如：175.0");
    m_heightBox->SetHeight(36.0f);
    m_heightBox->SetPadding(8.0f, 6.0f);
    m_heightBox->SetCornerRadius(4.0f);
    m_heightBox->SetBackground(Color(35, 40, 50, 255));
    m_heightBox->SetForeground(Colors::White);

    m_weightBox = std::make_shared<cppui::controls::TextBox>("例如：68.5");
    m_weightBox->SetHeight(36.0f);
    m_weightBox->SetPadding(8.0f, 6.0f);
    m_weightBox->SetCornerRadius(4.0f);
    m_weightBox->SetBackground(Color(35, 40, 50, 255));
    m_weightBox->SetForeground(Colors::White);

    m_resultValueLabel = std::make_shared<cppui::controls::Label>("BMI：--");
    m_resultValueLabel->SetFontSize(24.0f);
    m_resultValueLabel->SetFontWeight(cppui::text::FontWeight::Bold);
    m_resultValueLabel->SetForeground(Color(80, 200, 255, 255));

    m_resultCategoryLabel = std::make_shared<cppui::controls::Label>("體位判定：--");
    m_resultCategoryLabel->SetFontSize(15.0f);
    m_resultCategoryLabel->SetFontWeight(cppui::text::FontWeight::SemiBold);
    m_resultCategoryLabel->SetForeground(Colors::White);

    m_adviceLabel = std::make_shared<cppui::controls::Label>("請輸入身高與體重，並點擊「計算 BMI」");
    m_adviceLabel->SetFontSize(13.0f);
    m_adviceLabel->SetForeground(Color(180, 185, 195, 255));

    m_calcButton = std::make_shared<cppui::controls::Button>("計算 BMI");
    m_calcButton->SetHeight(38.0f);
    m_calcButton->SetBackground(Color(45, 120, 220, 255));
    m_calcButton->SetForeground(Colors::White);
    m_calcButton->SetCornerRadius(6.0f);
    m_calcButton->OnClick([this]() {
        CalculateBmi();
    });

    m_resetButton = std::make_shared<cppui::controls::Button>("重設清除");
    m_resetButton->SetHeight(38.0f);
    m_resetButton->SetBackground(Color(65, 70, 80, 255));
    m_resetButton->SetForeground(Colors::White);
    m_resetButton->SetCornerRadius(6.0f);
    m_resetButton->OnClick([this]() {
        ResetFields();
    });

    SetContent(
        VStack({
            Label("身體質量指數 (BMI) 計算機")
                .FontSize(20.0f)
                .FontWeight(cppui::text::FontWeight::Bold)
                .Foreground(Colors::White),

            Label("輸入您的身高與體重，快速掌握健康體態數值")
                .FontSize(12.0f)
                .Foreground(Color(160, 165, 175, 255)),

            Label("身高 (公分 cm)：")
                .FontSize(14.0f)
                .Foreground(Color(220, 225, 235, 255))
                .Margin(0.0f, 10.0f, 0.0f, 2.0f),

            UIBuilder<cppui::controls::TextBox>(m_heightBox),

            Label("體重 (公斤 kg)：")
                .FontSize(14.0f)
                .Foreground(Color(220, 225, 235, 255))
                .Margin(0.0f, 10.0f, 0.0f, 2.0f),

            UIBuilder<cppui::controls::TextBox>(m_weightBox),

            HStack({
                UIBuilder<cppui::controls::Button>(m_calcButton).Width(160.0f),
                UIBuilder<cppui::controls::Button>(m_resetButton).Width(100.0f)
            }).Spacing(12.0f).Margin(0.0f, 14.0f, 0.0f, 10.0f),

            Border(
                VStack({
                    UIBuilder<cppui::controls::Label>(m_resultValueLabel),
                    UIBuilder<cppui::controls::Label>(m_resultCategoryLabel),
                    UIBuilder<cppui::controls::Label>(m_adviceLabel)
                }).Spacing(6.0f)
            )
            .Background(Color(30, 34, 42, 255))
            .BorderBrush(Color(60, 68, 82, 255))
            .BorderThickness(1.0f)
            .CornerRadius(8.0f)
            .Padding(Thickness(14.0f))
        })
        .Padding(Thickness(24.0f))
        .Spacing(6.0f)
    );

    if (populateSample) {
        m_heightBox->SetText("175.0");
        m_weightBox->SetText("68.5");
        CalculateBmi();
    }
}

/// <summary>
/// 執行 BMI 計算並更新介面顯示。
/// </summary>
void BmiForm::CalculateBmi() {
    const compat::string_view heightStr = m_heightBox->GetText();
    const compat::string_view weightStr = m_weightBox->GetText();

    const auto heightExp = compat::parse<float>(heightStr);
    const auto weightExp = compat::parse<float>(weightStr);

    if (!heightExp.has_value() || !weightExp.has_value()) {
        m_resultValueLabel->SetText("BMI：--");
        m_resultValueLabel->SetForeground(cppui::core::Color(255, 100, 100, 255));
        m_resultCategoryLabel->SetText("體位判定：輸入格式錯誤");
        m_resultCategoryLabel->SetForeground(cppui::core::Color(255, 100, 100, 255));
        m_adviceLabel->SetText("請輸入有效的數值（例如身高 175、體重 68.5）。");
        return;
    }

    const float heightCm = heightExp.value();
    const float weightKg = weightExp.value();

    if (heightCm <= 50.0f || heightCm >= 250.0f || weightKg <= 10.0f || weightKg >= 300.0f) {
        m_resultValueLabel->SetText("BMI：--");
        m_resultValueLabel->SetForeground(cppui::core::Color(255, 100, 100, 255));
        m_resultCategoryLabel->SetText("體位判定：數值超出常理範圍");
        m_resultCategoryLabel->SetForeground(cppui::core::Color(255, 100, 100, 255));
        m_adviceLabel->SetText("身高應介於 50~250 cm，體重應介於 10~300 kg。");
        return;
    }

    const float heightM = heightCm / 100.0f;
    const float bmi = weightKg / (heightM * heightM);

    m_resultValueLabel->SetText(compat::format("BMI：{:.1f}", bmi));

    if (bmi < 18.5f) {
        m_resultValueLabel->SetForeground(cppui::core::Color(255, 193, 7, 255));
        m_resultCategoryLabel->SetText("體位判定：體重過輕 (Underweight)");
        m_resultCategoryLabel->SetForeground(cppui::core::Color(255, 193, 7, 255));
        m_adviceLabel->SetText("建議適度加強營養攝取與阻力訓練，提升健康體適能。");
    } else if (bmi < 24.0f) {
        m_resultValueLabel->SetForeground(cppui::core::Color(76, 175, 80, 255));
        m_resultCategoryLabel->SetText("體位判定：正常範圍 (Normal weight)");
        m_resultCategoryLabel->SetForeground(cppui::core::Color(76, 175, 80, 255));
        m_adviceLabel->SetText("太棒了！請繼續保持均衡飲食與規律運動的生活型態。");
    } else if (bmi < 27.0f) {
        m_resultValueLabel->SetForeground(cppui::core::Color(255, 152, 0, 255));
        m_resultCategoryLabel->SetText("體位判定：過重 (Overweight)");
        m_resultCategoryLabel->SetForeground(cppui::core::Color(255, 152, 0, 255));
        m_adviceLabel->SetText("建議注意熱量攝取，適量增加有氧與心肺活動。");
    } else {
        m_resultValueLabel->SetForeground(cppui::core::Color(244, 67, 54, 255));
        m_resultCategoryLabel->SetText("體位判定：肥胖 (Obese)");
        m_resultCategoryLabel->SetForeground(cppui::core::Color(244, 67, 54, 255));
        m_adviceLabel->SetText("建議諮詢專業醫師或營養師，規劃健康減重生活對策。");
    }
}

/// <summary>
/// 重設所有輸入方塊與結果標籤。
/// </summary>
void BmiForm::ResetFields() {
    m_heightBox->SetText("");
    m_weightBox->SetText("");
    m_resultValueLabel->SetText("BMI：--");
    m_resultValueLabel->SetForeground(cppui::core::Color(80, 200, 255, 255));
    m_resultCategoryLabel->SetText("體位判定：--");
    m_resultCategoryLabel->SetForeground(cppui::core::Colors::White);
    m_adviceLabel->SetText("請輸入身高與體重，並點擊「計算 BMI」");
}

} // namespace bmi_calculator
