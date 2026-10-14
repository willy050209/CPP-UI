# BMI 健康指數計算機教學範例 (BMI Calculator)

本範例展示如何運用 **CPPUI** 打造一個具備生產級質感的卡片式現代深色風格桌面應用程式。涵蓋文字輸入解析、非法數值邊界校驗、BMI 醫學區間計算以及根據結果動態更新色彩徽章視覺回饋。

---

## 1. 成果預覽 (Preview)

![BMI 計算機視窗預覽](../../docs/images/bmi_calculator_preview.png)

---

## 2. 範例定位與功能特色

- **現代卡片式深色風格**：採用微軟 Fluent / macOS 風格之圓角卡片邊框 (`Border`) 與層次色彩代幣。
- **即時數值運算與輸入防護**：捕捉身高與體重字串輸入，進行嚴格的浮點數轉型防護與除以零邊界校驗。
- **語意狀態回饋**：依據 WHO 體重標準（過輕、正常、過重、肥胖），動態切換結果面板的色彩與提示文字。

---

## 3. 核心技術與演算法

### 3.1 BMI 計算純函數 (SoC & Pure Functions)
業務邏輯獨立於 UI 渲染之外，保證純粹無副作用：

```cpp
struct BmiResult {
    float bmi = 0.0f;
    std::string category;
    core::Color badgeColor;
};

static BmiResult CalculateBmi(float heightCm, float weightKg) {
    if (heightCm <= 0.0f || weightKg <= 0.0f) {
        return { 0.0f, "數值無效", core::Color(128, 128, 128) };
    }
    float heightM = heightCm / 100.0f;
    float bmi = weightKg / (heightM * heightM);

    if (bmi < 18.5f) {
        return { bmi, "體重過輕 (Underweight)", core::Color(59, 130, 246) }; // 藍色
    } else if (bmi < 24.0f) {
        return { bmi, "體重正常 (Normal weight)", core::Color(34, 197, 94) }; // 綠色
    } else if (bmi < 27.0f) {
        return { bmi, "體重過重 (Overweight)", core::Color(234, 179, 8) };   // 黃色
    } else {
        return { bmi, "肥胖狀態 (Obese)", core::Color(239, 68, 68) };        // 紅色
    }
}
```

### 3.2 宣告式表單構建
透過 `TextBox` 接收使用者鍵盤輸入，點擊計算按鈕後即時更新結果卡片：

```cpp
auto btnCalc = Button("計算 BMI 指數")
    .Primary()
    .OnClick([this]() {
        float h = ParseFloat(m_txtHeight->GetText());
        float w = ParseFloat(m_txtWeight->GetText());
        auto result = CalculateBmi(h, w);
        UpdateResultUI(result);
    })
    .Build();
```

---

## 4. 建置與執行方式

### 透過 CMake 建置
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target bmi_calculator
```

### 執行範例
```powershell
# Windows
.\build\bin\Release\bmi_calculator.exe

# Linux
./build/bin/bmi_calculator
```
