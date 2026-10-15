/**
 * @file PictureBox.hpp
 * @brief WinForms 風格圖片顯示控制項，支援 5 種尺寸佈局模式
 */

#pragma once

#include <memory>
#include "compat/Compat.hpp"
#include "ui/controls/Control.hpp"
#include "ui/graphics/Bitmap.hpp"
#include "ui/graphics/GraphicsTypes.hpp"

namespace cppui {
namespace controls {

/// <summary>
/// 圖片顯示控制項 (PictureBox)
/// 支援 Normal, StretchImage, AutoSize, CenterImage, Zoom 五種尺寸模式與 Fluent DSL。
/// </summary>
class PictureBox : public Control {
public:
    /// <summary>預設建構子。</summary>
    PictureBox();

    /// <summary>
    /// 以指定影像建構 PictureBox。
    /// </summary>
    /// <param name="image">影像 Bitmap 共享指標。</param>
    explicit PictureBox(std::shared_ptr<graphics::Bitmap> image);

    /// <summary>解構子。</summary>
    ~PictureBox() override = default;

    /// <summary>取得目前顯示之影像指標。</summary>
    /// <returns>Bitmap 共享指標。</returns>
    COMPAT_NODISCARD std::shared_ptr<graphics::Bitmap> GetImage() const;

    /// <summary>
    /// 設定顯示之影像。
    /// </summary>
    /// <param name="image">新影像 Bitmap 共享指標。</param>
    void SetImage(std::shared_ptr<graphics::Bitmap> image);

    /// <summary>取得圖片尺寸佈局模式。</summary>
    /// <returns>PictureBoxSizeMode 列舉值。</returns>
    COMPAT_NODISCARD graphics::PictureBoxSizeMode GetSizeMode() const;

    /// <summary>
    /// 設定圖片尺寸佈局模式。
    /// </summary>
    /// <param name="mode">新尺寸模式。</param>
    void SetSizeMode(graphics::PictureBoxSizeMode mode);

    /// <summary>
    /// 取得影像檔案路徑 (ImageLocation)。
    /// </summary>
    /// <returns>檔案路徑字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetImageLocation() const noexcept { return m_imageLocation; }

    /// <summary>
    /// 設定影像檔案路徑並嘗試載入。
    /// </summary>
    /// <param name="path">檔案路徑字串視圖。</param>
    void SetImageLocation(compat::string_view path);

    /// <summary>
    /// 從指定的影像檔案路徑載入影像。
    /// </summary>
    /// <param name="path">檔案路徑字串視圖。</param>
    /// <returns>若載入成功傳回 true，失敗傳回 false。</returns>
    bool Load(compat::string_view path);

protected:
    /// <summary>度量計算控制項期望尺寸。</summary>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>配置控制項內部排列。</summary>
    void ArrangeOverride(core::Size finalSize) override;

    /// <summary>繪製控制項畫面。</summary>
    void OnRender(rendering::DrawingContext& dc) override;

private:
    std::shared_ptr<graphics::Bitmap> m_image;
    graphics::PictureBoxSizeMode      m_sizeMode{ graphics::PictureBoxSizeMode::Normal };
    std::string                       m_imageLocation;
};

} // namespace controls
} // namespace cppui
