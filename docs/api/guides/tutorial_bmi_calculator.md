# 建立我的第一個 BMI 計算機程式 (Tutorial: Building a BMI Calculator)

身體質量指數 (Body Mass Index, 簡稱 **BMI**) 是國際上常用於評估個人體位是否處於健康範圍的重要指標。本教學將帶領您一步步使用 **CPPUI** 打造一個具備現代深色卡片介面、即時數值驗證、動態色彩回饋與重設功能的互動式 BMI 桌面應用程式。

---

## 1. 成果預覽 (Preview)

完成本教學後，您將運行出如下圖所示的真實視窗應用程式：

![BMI 健康計算機預覽](../../images/bmi_calculator_preview.png)

---

## 2. 核心功能與健康標準公式

### BMI 數學計算公式

$$\text{BMI} = \frac{\text{體重 (公斤 kg)}}{(\text{身高 (公尺 m)})^2} = \frac{\text{體重 (kg)}}{\left(\frac{\text{身高 (cm)}}{100}\right)^2}$$

### 衛福部體位判定標準與 UI 回饋色彩

| BMI 範圍 | 體位分級 | 狀態提示 | UI 主題色彩 |
| :--- | :--- | :--- | :--- |
| $\text{BMI} < 18.5$ | **體重過輕 (Underweight)** | 建議適度加強營養攝取與阻力訓練，提升健康體適能。 | 琥珀黃 (`#FFC107`) |
| $18.5 \le \text{BMI} < 24.0$ | **正常範圍 (Normal weight)** | 太棒了！請繼續保持均衡飲食與規律運動的生活型態。 | 翠綠 (`#4CAF50`) |
| $24.0 \le \text{BMI} < 27.0$ | **體位過重 (Overweight)** | 建議注意熱量攝取，適量增加有氧與心肺活動。 | 橘黃 (`#FF9800`) |
| $\text{BMI} \ge 27.0$ | **肥胖 (Obese)** | 建議諮詢專業醫師或營養師，規劃健康減重生活對策。 | 鮮紅 (`#F44336`) |

---

## 3. 專案目錄結構

我們建議將專案結構組織為清晰的模組化目錄：

```text
examples/bmi_calculator/
├── BmiForm.hpp          # 視窗表單類別宣告與成員定義
├── BmiForm.cpp          # Fluent DSL 佈局與計算業務邏輯實作
└── main.cpp             # 應用程式進入點 Application::Run<BmiForm>()
```

---

## 4. 步驟一：表單介面宣告 (BmiForm.hpp)

在 `BmiForm.hpp` 中，我們繼承 [`cppui::app::Form`](../classes/app_form.md)，並宣告輸入方塊 (`TextBox`)、操作按鈕 (`Button`) 以及顯示 BMI 數值與健康建議的文字標籤 (`Label`)：

```cpp
/*
 * File: BmiForm.hpp
 * Purpose: 宣告 BmiForm 互動式健康計算機視窗類別
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
```

---

## 5. 步驟二：Fluent DSL 介面佈局實作 (BmiForm.cpp)

在建構子中，我們透過 CPPUI 的 **Fluent RAD DSL**，使用垂直容器 `VStack`、水平容器 `HStack` 與卡片邊框 `Border` 快速堆疊出階層視覺元件：

```cpp
/*
 * File: BmiForm.cpp
 * Purpose: 實作 BmiForm 佈局與 BMI 計算邏輯
 */

#include "BmiForm.hpp"
#include "ui/dsl/Fluent.hpp"
#include "compat/compat.hpp"

namespace bmi_calculator {

BmiForm::BmiForm(bool populateSample) {
    using namespace cppui::dsl;
    using namespace cppui::core;

    SetTitle("BMI 健康計算機 - CPPUI");
    SetFormSize(420, 520);

    // 1. 初始化身高與體重輸入框
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

    // 2. 初始化結果回饋標籤
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

    // 3. 初始化操作按鈕並綁定點擊事件回呼
    m_calcButton = std::make_shared<cppui::controls::Button>("計算 BMI");
    m_calcButton->SetHeight(38.0f);
    m_calcButton->SetBackground(Color(45, 120, 220, 255));
    m_calcButton->SetForeground(Colors::White);
    m_calcButton->SetCornerRadius(6.0f);
    m_calcButton->OnClick([this]() { CalculateBmi(); });

    m_resetButton = std::make_shared<cppui::controls::Button>("重設清除");
    m_resetButton->SetHeight(38.0f);
    m_resetButton->SetBackground(Color(65, 70, 80, 255));
    m_resetButton->SetForeground(Colors::White);
    m_resetButton->SetCornerRadius(6.0f);
    m_resetButton->OnClick([this]() { ResetFields(); });

    // 4. 宣告式建立整體介面樹
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

            // 結果顯示卡片
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
```

---

## 6. 步驟三：數值解析與防禦性錯誤處理 (CalculateBmi & ResetFields)

遵循 **Fail-Fast 原則** 與安全性防禦規範，我們使用 [`compat::parse<float>`](../namespaces/compat.md) 進行零例外、無文化特性干擾的數值轉換，並對非法字串或荒謬數值進行即時防護：

```cpp
void BmiForm::CalculateBmi() {
    const compat::string_view heightStr = m_heightBox->GetText();
    const compat::string_view weightStr = m_weightBox->GetText();

    // 1. 嘗試解析浮點數
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

    // 2. 常理範圍校驗
    if (heightCm <= 50.0f || heightCm >= 250.0f || weightKg <= 10.0f || weightKg >= 300.0f) {
        m_resultValueLabel->SetText("BMI：--");
        m_resultValueLabel->SetForeground(cppui::core::Color(255, 100, 100, 255));
        m_resultCategoryLabel->SetText("體位判定：數值超出常理範圍");
        m_resultCategoryLabel->SetForeground(cppui::core::Color(255, 100, 100, 255));
        m_adviceLabel->SetText("身高應介於 50~250 cm，體重應介於 10~300 kg。");
        return;
    }

    // 3. 執行標準 BMI 運算
    const float heightM = heightCm / 100.0f;
    const float bmi = weightKg / (heightM * heightM);

    m_resultValueLabel->SetText(compat::format("BMI：{:.1f}", bmi));

    // 4. 動態健康分級與色彩呈現
    if (bmi < 18.5f) {
        m_resultValueLabel->SetForeground(cppui::core::Color(255, 193, 7, 255)); // 琥珀黃
        m_resultCategoryLabel->SetText("體位判定：體重過輕 (Underweight)");
        m_resultCategoryLabel->SetForeground(cppui::core::Color(255, 193, 7, 255));
        m_adviceLabel->SetText("建議適度加強營養攝取與阻力訓練，提升健康體適能。");
    } else if (bmi < 24.0f) {
        m_resultValueLabel->SetForeground(cppui::core::Color(76, 175, 80, 255)); // 翠綠
        m_resultCategoryLabel->SetText("體位判定：正常範圍 (Normal weight)");
        m_resultCategoryLabel->SetForeground(cppui::core::Color(76, 175, 80, 255));
        m_adviceLabel->SetText("太棒了！請繼續保持均衡飲食與規律運動的生活型態。");
    } else if (bmi < 27.0f) {
        m_resultValueLabel->SetForeground(cppui::core::Color(255, 152, 0, 255)); // 橘黃
        m_resultCategoryLabel->SetText("體位判定：過重 (Overweight)");
        m_resultCategoryLabel->SetForeground(cppui::core::Color(255, 152, 0, 255));
        m_adviceLabel->SetText("建議注意熱量攝取，適量增加有氧與心肺活動。");
    } else {
        m_resultValueLabel->SetForeground(cppui::core::Color(244, 67, 54, 255)); // 鮮紅
        m_resultCategoryLabel->SetText("體位判定：肥胖 (Obese)");
        m_resultCategoryLabel->SetForeground(cppui::core::Color(244, 67, 54, 255));
        m_adviceLabel->SetText("建議諮詢專業醫師或營養師，規劃健康減重生活對策。");
    }
}

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
```

---

## 7. 步驟四：應用程式啟動入口 (main.cpp)

在 `main.cpp` 中以 `Application::Run<bmi_calculator::BmiForm>()` 啟動：

```cpp
/*
 * File: main.cpp
 * Purpose: BMI 計算機應用程式進入點
 */

#include "ui/app/Application.hpp"
#include "BmiForm.hpp"

int main() {
    return cppui::app::Application::Run<bmi_calculator::BmiForm>();
}
```

---

## 8. 編譯與執行 (Build & Run)

在專案根目錄下執行以下指令完成編譯與運行：

```powershell
# 1. 透過 CMake 編譯 bmi_calculator 目標
cmake --build build --config Debug --target bmi_calculator

# 2. 啟動 BMI 計算機
.\build\bin\Debug\bmi_calculator.exe
```

---

## 9. 重點技術回顧

1. **零成本抽象 (Zero-cost Abstraction)**：透過 `UIBuilder` 包裝既有 `shared_ptr` 控制項，既保有了對實例指針的直接存取能力，又享有 Fluent DSL 的流式排版語法。
2. **型別安全格式化與解析**：`compat::parse<float>` 與 `compat::format("{:.1f}", bmi)` 保證跨平台無 locale 依賴，精確又安全。
3. **動態即時回饋**：透過修改標籤的 `SetForeground()` 色彩，直接給予使用者直觀的健康狀態色碼引導。

---

## 10. 下一步推薦 (Next Tutorial)

學會了表單與控制項互動後，您可以進一步探索 CPPUI 強大的 2D 向量繪圖與影像處理引擎：

👉 **[2D 圖形繪圖與影像濾鏡教學 (Tutorial: Graphics & Drawing)](tutorial_graphics_drawing.md)** —— 學習使用 `Graphics` 繪製直線、圓弧、三次貝茲曲線、自訂幾何路徑，並使用 `ImageFilters` 實現高效能灰階轉換！
