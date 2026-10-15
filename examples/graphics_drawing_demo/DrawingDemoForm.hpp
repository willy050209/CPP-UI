/*
 * File: DrawingDemoForm.hpp
 * Purpose: Declares DrawingDemoForm demonstrating 2D vector drawing, bitmap drawing,
 *          and Grayscale image filter effects.
 */

#pragma once

#include <memory>
#include <cstdint>
#include "compat/Compat.hpp"
#include "ui/app/Form.hpp"
#include "ui/controls/PictureBox.hpp"
#include "ui/controls/Label.hpp"
#include "ui/controls/Button.hpp"
#include "ui/graphics/Bitmap.hpp"
#include "ui/graphics/GraphicsPath.hpp"
#include "ui/graphics/ImageFilters.hpp"

namespace graphics_drawing_demo {

/// <summary>
/// 向量繪圖與幾何計算輔助類別 (SoC 純函數靜態隔離)
/// </summary>
class DrawingDemoMath {
public:
    /// <summary>
    /// 建立正五角星幾何路徑 (GraphicsPath)。
    /// </summary>
    /// <param name="center">星形中心點。</param>
    /// <param name="outerRadius">外徑半徑像素值。</param>
    /// <param name="innerRadius">內徑半徑像素值。</param>
    /// <param name="points">角數 (預設 5)。</param>
    /// <returns>五角星封閉幾何路徑。</returns>
    COMPAT_NODISCARD static cppui::graphics::GraphicsPath CreateStarPath(
        cppui::core::Point center,
        float outerRadius,
        float innerRadius,
        uint32_t points = 5);

    /// <summary>
    /// 生成展示用之印記徽章點陣圖，示範動態繪製與圖片載入。
    /// </summary>
    /// <param name="size">徽章正方形邊長尺寸。</param>
    /// <returns>點陣圖共享指標。</returns>
    COMPAT_NODISCARD static std::shared_ptr<cppui::graphics::Bitmap> CreateSampleBadge(uint32_t size);
};

/// <summary>
/// 2D 向量繪圖、圖片繪製與灰階濾鏡展示視窗表單
/// </summary>
class DrawingDemoForm : public cppui::app::Form {
public:
    /// <summary>
    /// 建構 DrawingDemoForm 並繪製初始畫布與介面元件。
    /// </summary>
    /// <param name="startGrayscale">是否初始即套用灰階濾鏡。</param>
    explicit DrawingDemoForm(bool startGrayscale = false);

    ~DrawingDemoForm() override = default;

private:
    /// <summary>繪製彩色主畫布內容。</summary>
    void RenderMasterScene();

    /// <summary>套用指定灰階轉換演算法。</summary>
    /// <param name="method">灰階轉換標準方法。</param>
    /// <param name="description">演算法說明字串。</param>
    void ApplyGrayscale(cppui::graphics::GrayscaleMethod method, compat::string_view description);

    /// <summary>還原為原始彩色影像。</summary>
    void ResetOriginal();

    std::shared_ptr<cppui::graphics::Bitmap>     m_originalBitmap;
    std::shared_ptr<cppui::graphics::Bitmap>     m_currentBitmap;
    std::shared_ptr<cppui::controls::PictureBox> m_pictureBox;
    std::shared_ptr<cppui::controls::Label>      m_statusLabel;
};

} // namespace graphics_drawing_demo
