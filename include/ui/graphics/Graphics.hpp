/**
 * @file Graphics.hpp
 * @brief GDI+ 風格統一繪圖排程與畫布控制中樞類別
 */

#pragma once

#include <memory>
#include <vector>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/graphics/GraphicsTypes.hpp"
#include "ui/graphics/Pen.hpp"
#include "ui/graphics/Brush.hpp"
#include "ui/graphics/GraphicsPath.hpp"
#include "ui/graphics/Bitmap.hpp"
#include "ui/text/Font.hpp"

namespace cppui {

namespace rendering {
class DrawingContext;
class DisplayList;
} // namespace rendering

namespace graphics {

/// <summary>
/// 統一 2D 向量繪圖與狀態管理排程器 (Graphics)
/// 封裝直線、幾何圖元、曲線路徑、文字與影像繪製，支援 UI DisplayList 錄製與離線記憶體影像渲染。
/// </summary>
class Graphics {
public:
    /// <summary>
    /// 從 UI DrawingContext 建構 Graphics 實例 (即時 UI 繪製模式)。
    /// </summary>
    /// <param name="dc">DrawingContext 引用。</param>
    /// <returns>Graphics 物件。</returns>
    COMPAT_NODISCARD static Graphics FromDrawingContext(rendering::DrawingContext& dc);

    /// <summary>
    /// 從記憶體點陣圖建構 Graphics 實例 (離線影像渲染模式)。
    /// </summary>
    /// <param name="image">目標 Bitmap 共享指標。</param>
    /// <returns>Graphics 物件。</returns>
    /// <exception cref="std::invalid_argument">若 image 為 nullptr 擲出例外。</exception>
    COMPAT_NODISCARD static Graphics FromImage(std::shared_ptr<Bitmap> image);

    /// <summary>
    /// 直接從 UI DrawingContext 建構。
    /// </summary>
    /// <param name="dc">DrawingContext 引用。</param>
    explicit Graphics(rendering::DrawingContext& dc);

    /// <summary>
    /// 直接從記憶體點陣圖建構。
    /// </summary>
    /// <param name="image">目標 Bitmap 共享指標。</param>
    /// <exception cref="std::invalid_argument">若 image 為 nullptr 擲出例外。</exception>
    explicit Graphics(std::shared_ptr<Bitmap> image);

    /// <summary>移動建構子。</summary>
    Graphics(Graphics&& other) noexcept;
    /// <summary>移動賦值運算子。</summary>
    Graphics& operator=(Graphics&& other) noexcept;

    // 禁止拷貝 (持有專屬畫布狀態)
    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;

    /// <summary>解構子，在離線影像模式下自動提交繪製命令。</summary>
    ~Graphics();

    /// <summary>
    /// 儲存當前繪圖畫布狀態 (變換矩陣、剪裁區域、品質設定)。
    /// </summary>
    /// <returns>狀態識別代碼。</returns>
    uint32_t Save();

    /// <summary>
    /// 回復先前儲存之繪圖畫布狀態。
    /// </summary>
    /// <param name="stateId">狀態識別代碼。</param>
    void Restore(uint32_t stateId);

    /// <summary>
    /// 套用平移幾何變換。
    /// </summary>
    /// <param name="dx">X 軸偏移量。</param>
    /// <param name="dy">Y 軸偏移量。</param>
    void TranslateTransform(float dx, float dy);

    /// <summary>
    /// 套用縮放幾何變換。
    /// </summary>
    /// <param name="sx">X 軸縮放比例。</param>
    /// <param name="sy">Y 軸縮放比例。</param>
    void ScaleTransform(float sx, float sy);

    /// <summary>
    /// 套用旋轉幾何變換。
    /// </summary>
    /// <param name="angleDegrees">旋轉角度 (度)。</param>
    void RotateTransform(float angleDegrees);

    /// <summary>重設變換矩陣為單位矩陣。</summary>
    void ResetTransform();

    /// <summary>
    /// 設定矩形剪裁區域。
    /// </summary>
    /// <param name="rect">剪裁矩形。</param>
    void SetClip(const core::Rect& rect);

    /// <summary>
    /// 設定幾何路徑剪裁區域。
    /// </summary>
    /// <param name="path">剪裁路徑。</param>
    void SetClip(const GraphicsPath& path);

    /// <summary>重設並清空剪裁區域。</summary>
    void ResetClip();

    /// <summary>取得當前有效剪裁邊界。</summary>
    /// <returns>剪裁邊界矩形。</returns>
    COMPAT_NODISCARD core::Rect GetClipBounds() const;

    /// <summary>設定平滑化反鋸齒模式。</summary>
    /// <param name="mode">SmoothingMode 列舉值。</param>
    void SetSmoothingMode(SmoothingMode mode);

    /// <summary>取得平滑化反鋸齒模式。</summary>
    /// <returns>目前 SmoothingMode。</returns>
    COMPAT_NODISCARD SmoothingMode GetSmoothingMode() const;

    /// <summary>設定影像縮放插值模式。</summary>
    /// <param name="mode">InterpolationMode 列舉值。</param>
    void SetInterpolationMode(InterpolationMode mode);

    /// <summary>取得影像縮放插值模式。</summary>
    /// <returns>目前 InterpolationMode。</returns>
    COMPAT_NODISCARD InterpolationMode GetInterpolationMode() const;

    /// <summary>
    /// 以指定色彩填滿清除整幅畫布。
    /// </summary>
    /// <param name="color">背景色彩。</param>
    void Clear(core::Color color);

    /// <summary>繪製線段。</summary>
    void DrawLine(const Pen& pen, core::Point pt1, core::Point pt2);
    /// <summary>繪製線段。</summary>
    void DrawLine(const Pen& pen, float x1, float y1, float x2, float y2);
    /// <summary>繪製相連線段序列。</summary>
    void DrawLines(const Pen& pen, compat::span<const core::Point> points);

    /// <summary>繪製矩形邊框。</summary>
    void DrawRectangle(const Pen& pen, const core::Rect& rect);
    /// <summary>繪製矩形邊框。</summary>
    void DrawRectangle(const Pen& pen, float x, float y, float width, float height);
    /// <summary>繪製多個矩形邊框。</summary>
    void DrawRectangles(const Pen& pen, compat::span<const core::Rect> rects);

    /// <summary>填滿矩形區域。</summary>
    void FillRectangle(const Brush& brush, const core::Rect& rect);
    /// <summary>填滿矩形區域。</summary>
    void FillRectangle(const Brush& brush, float x, float y, float width, float height);
    /// <summary>填滿多個矩形區域。</summary>
    void FillRectangles(const Brush& brush, compat::span<const core::Rect> rects);

    /// <summary>繪製圓角矩形邊框。</summary>
    void DrawRoundedRectangle(const Pen& pen, const core::Rect& rect, float radius);
    /// <summary>填滿圓角矩形內部。</summary>
    void FillRoundedRectangle(const Brush& brush, const core::Rect& rect, float radius);

    /// <summary>繪製橢圓邊框。</summary>
    void DrawEllipse(const Pen& pen, const core::Rect& rect);
    /// <summary>繪製橢圓邊框。</summary>
    void DrawEllipse(const Pen& pen, float x, float y, float width, float height);
    /// <summary>填滿橢圓內部。</summary>
    void FillEllipse(const Brush& brush, const core::Rect& rect);
    /// <summary>填滿橢圓內部。</summary>
    void FillEllipse(const Brush& brush, float x, float y, float width, float height);

    /// <summary>繪製圓弧。</summary>
    void DrawArc(const Pen& pen, const core::Rect& rect, float startAngle, float sweepAngle);
    /// <summary>繪製扇形邊框。</summary>
    void DrawPie(const Pen& pen, const core::Rect& rect, float startAngle, float sweepAngle);
    /// <summary>填滿扇形內部。</summary>
    void FillPie(const Brush& brush, const core::Rect& rect, float startAngle, float sweepAngle);

    /// <summary>繪製多邊形輪廓。</summary>
    void DrawPolygon(const Pen& pen, compat::span<const core::Point> points);
    /// <summary>填滿多邊形內部。</summary>
    void FillPolygon(const Brush& brush, compat::span<const core::Point> points, FillMode fillMode = FillMode::Alternate);

    /// <summary>繪製三次貝茲曲線。</summary>
    void DrawBezier(const Pen& pen, core::Point pt1, core::Point pt2, core::Point pt3, core::Point pt4);
    /// <summary>繪製連續貝茲曲線序列。</summary>
    void DrawBeziers(const Pen& pen, compat::span<const core::Point> points);
    /// <summary>繪製平滑曲線。</summary>
    void DrawCurve(const Pen& pen, compat::span<const core::Point> points);

    /// <summary>描繪幾何路徑輪廓。</summary>
    void DrawPath(const Pen& pen, const GraphicsPath& path);
    /// <summary>填滿幾何路徑內部。</summary>
    void FillPath(const Brush& brush, const GraphicsPath& path);

    /// <summary>繪製點陣影像。</summary>
    void DrawImage(std::shared_ptr<Bitmap> image, const core::Rect& destRect);
    /// <summary>裁切並縮放繪製點陣影像。</summary>
    void DrawImage(std::shared_ptr<Bitmap> image, const core::Rect& destRect, const core::Rect& srcRect);
    /// <summary>繪製原寸點陣影像。</summary>
    void DrawImage(std::shared_ptr<Bitmap> image, float x, float y);
    /// <summary>繪製指定大小點陣影像。</summary>
    void DrawImage(std::shared_ptr<Bitmap> image, float x, float y, float width, float height);
    /// <summary>以原尺寸無縮放繪製影像。</summary>
    void DrawImageUnscaled(std::shared_ptr<Bitmap> image, core::Point origin);

    /// <summary>繪製字串文字。</summary>
    void DrawString(compat::string_view text, const text::FontDescriptor& font, const Brush& brush, core::Point origin);
    /// <summary>在指定佈局矩形內繪製文字。</summary>
    void DrawString(compat::string_view text, const text::FontDescriptor& font, const Brush& brush, const core::Rect& layoutRect);
    /// <summary>度量字串顯示尺寸。</summary>
    COMPAT_NODISCARD core::Size MeasureString(compat::string_view text, const text::FontDescriptor& font);

    /// <summary>提交並執行所有待渲染命令。</summary>
    void Flush();

    /// <summary>
    /// RAII 畫布繪圖狀態保護守衛 (StateScope)
    /// </summary>
    class StateScope {
    public:
        /// <summary>建構時自動保存狀態。</summary>
        explicit StateScope(Graphics& g) : m_graphics(g), m_stateId(g.Save()) {}
        /// <summary>解構時自動還原狀態。</summary>
        ~StateScope() { m_graphics.Restore(m_stateId); }
    private:
        Graphics& m_graphics;
        uint32_t  m_stateId;
    };

private:
    struct Impl;
    compat::propagate_const<std::unique_ptr<Impl>> m_impl;
};

} // namespace graphics
} // namespace cppui
