/*
 * File: CardioidForm.hpp
 * Purpose: Declares CardioidForm demonstrating GDI+ style Graphics, GraphicsPath,
 *          LinearGradientBrush, Pen, Bitmap, and PictureBox integration.
 */

#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include "ui/app/Form.hpp"
#include "ui/controls/PictureBox.hpp"
#include "ui/graphics/Bitmap.hpp"
#include "ui/graphics/GraphicsPath.hpp"
#include "ui/graphics/Brush.hpp"
#include "ui/graphics/Pen.hpp"
#include "ui/reactive/Signal.hpp"

namespace drawing_cardioid_example {

/// <summary>
/// 心臟線幾何曲線演算法純函數輔助類別 (SoC 純函數靜態隔離)
/// </summary>
class CardioidMath {
public:
    /// <summary>
    /// 生成古典心臟線幾何路徑 (Classical Cardioid)。
    /// 參數方程：x = cx + a * (2*cos(t) - cos(2*t)), y = cy - a * (2*sin(t) - sin(2*t))。
    /// </summary>
    /// <param name="center">心臟線中心錨點。</param>
    /// <param name="scale">縮放尺度係數。</param>
    /// <param name="samples">採樣頂點數量 (預設 360)。</param>
    /// <returns>已閉合之 GraphicsPath 路徑實例。</returns>
    COMPAT_NODISCARD static cppui::graphics::GraphicsPath GenerateClassicalCardioid(
        cppui::core::Point center, float scale, uint32_t samples = 360);

    /// <summary>
    /// 生成浪漫心形曲線幾何路徑 (Parametric Heart Curve)。
    /// 參數方程：x = cx + a * 16*sin^3(t), y = cy - a * (13*cos(t) - 5*cos(2*t) - 2*cos(3*t) - cos(4*t))。
    /// </summary>
    /// <param name="center">心形幾何中心。</param>
    /// <param name="scale">縮放尺度係數。</param>
    /// <param name="samples">採樣頂點數量 (預設 360)。</param>
    /// <returns>已閉合之 GraphicsPath 路徑實例。</returns>
    COMPAT_NODISCARD static cppui::graphics::GraphicsPath GenerateHeartCurve(
        cppui::core::Point center, float scale, uint32_t samples = 360);
};

/// <summary>
/// 曲線幾何型態列舉
/// </summary>
enum class CurveType : uint8_t {
    ClassicalCardioid,
    RomanticHeart
};

/// <summary>
/// 漸層色盤配置列舉
/// </summary>
enum class GradientScheme : uint8_t {
    SunsetCrimson, // 日落暮色 (Crimson -> RoyalBlue)
    OceanBreeze,   // 蔚藍海岸 (DeepSkyBlue -> Indigo)
    EmeraldForest  // 翡翠森林 (SpringGreen -> DarkSlateGray)
};

/// <summary>
/// 心臟線與漸層藝術繪圖示範視窗表單
/// </summary>
class CardioidForm : public cppui::app::Form {
public:
    /// <summary>
    /// 建構 CardioidForm 並初始化繪圖緩衝區與介面佈局。
    /// </summary>
    CardioidForm();

    /// <summary>
    /// 解構 CardioidForm 實例。
    /// </summary>
    ~CardioidForm() override = default;

    /// <summary>
    /// 依據當前選取之曲線模式與漸層配置重新光柵化繪圖。
    /// </summary>
    void RefreshArtwork();

protected:
    /// <summary>
    /// 視窗加載生命週期事件回呼。
    /// </summary>
    void OnLoad() override;

private:
    CurveType                              m_curveType{CurveType::ClassicalCardioid};
    GradientScheme                         m_gradientScheme{GradientScheme::SunsetCrimson};
    float                                  m_strokeWidth{2.5f};
    std::shared_ptr<cppui::graphics::Bitmap>    m_bitmap;
    std::shared_ptr<cppui::controls::PictureBox> m_pictureBox;

    cppui::reactive::Signal<std::string>   m_infoText{"目前曲線: 古典心臟線 (Classical Cardioid) | 漸層: 日落暮色"};
};

} // namespace drawing_cardioid_example
