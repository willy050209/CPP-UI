/**
 * @file DrawingContext.hpp
 * @brief 繪圖上下文錄製介面，將圖元繪製命令序列化至 DisplayList
 */

#pragma once

#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/text/Font.hpp"
#include "ui/text/TextLayout.hpp"
#include "ui/graphics/GraphicsPath.hpp"
#include "ui/graphics/Brush.hpp"

namespace cppui {
namespace rendering {

/// <summary>
/// 公開畫布錄製抽象介面 (DrawingContext)
/// UI 控制項在此產生純量繪製命令，將操作序列化寫入 DisplayList，不直接觸發 GPU/光柵化。
/// </summary>
class DrawingContext {
public:
    /// <summary>
    /// 建構畫布錄製上下文。
    /// </summary>
    /// <param name="displayList">目標顯示清單引用。</param>
    explicit DrawingContext(DisplayList& displayList) noexcept
        : m_displayList(displayList) {}

    /// <summary>解構子。</summary>
    ~DrawingContext() = default;

    /// <summary>
    /// 繪製填色矩形。
    /// </summary>
    /// <param name="rect">矩形邊界。</param>
    /// <param name="color">填色色彩。</param>
    void DrawRectangle(const core::Rect& rect, core::Color color) {
        if (rect.IsEmpty() || color.a == 0) return;
        core::Rect offsetRect(rect.x + m_currentOffset.x, rect.y + m_currentOffset.y, rect.width, rect.height);
        DrawRectOp op{ offsetRect, color };
        m_displayList.PushOp(DrawOpType::DrawRectangle, op);
    }

    /// <summary>
    /// 繪製圓角矩形。
    /// </summary>
    /// <param name="rect">矩形邊界。</param>
    /// <param name="radius">圓角半徑像素值。</param>
    /// <param name="color">填色色彩。</param>
    void DrawRoundedRectangle(const core::Rect& rect, float radius, core::Color color) {
        if (rect.IsEmpty() || color.a == 0) return;
        core::Rect offsetRect(rect.x + m_currentOffset.x, rect.y + m_currentOffset.y, rect.width, rect.height);
        DrawRoundedRectOp op{ offsetRect, radius, color };
        m_displayList.PushOp(DrawOpType::DrawRoundedRectangle, op);
    }

    /// <summary>
    /// 繪製線段。
    /// </summary>
    /// <param name="p1">起點座標。</param>
    /// <param name="p2">終點座標。</param>
    /// <param name="color">線條色彩。</param>
    /// <param name="strokeWidth">線條粗細，預設為 1.0f。</param>
    void DrawLine(core::Point p1, core::Point p2, core::Color color, float strokeWidth = 1.0f) {
        if (color.a == 0 || strokeWidth <= 0.0f) return;
        core::Point op1(p1.x + m_currentOffset.x, p1.y + m_currentOffset.y);
        core::Point op2(p2.x + m_currentOffset.x, p2.y + m_currentOffset.y);
        DrawLineOp op{ op1, op2, color, strokeWidth };
        m_displayList.PushOp(DrawOpType::DrawLine, op);
    }

    /// <summary>
    /// 繪製單行純文字。
    /// </summary>
    /// <param name="text">文字字串視圖。</param>
    /// <param name="origin">文字基準原點。</param>
    /// <param name="font">字型描述項。</param>
    /// <param name="color">文字色彩。</param>
    void DrawText(compat::string_view text, core::Point origin, const text::FontDescriptor& font, core::Color color) {
        if (text.empty() || color.a == 0) return;
        DrawTextOp op;
        op.origin = core::Point(origin.x + m_currentOffset.x, origin.y + m_currentOffset.y);
        op.color = color;
        op.fontSize = font.GetSize();
        op.fontWeight = static_cast<uint16_t>(font.GetWeight());
        op.fontStyle = static_cast<uint8_t>(font.GetStyle());
        op.reserved = 0;
        op.familyLength = static_cast<uint32_t>(font.GetFamily().size());
        op.textLength = static_cast<uint32_t>(text.size());

        size_t totalExtra = op.familyLength + op.textLength;
        std::vector<char> extra(totalExtra);
        if (op.familyLength > 0) {
            std::memcpy(extra.data(), font.GetFamily().data(), op.familyLength);
        }
        if (op.textLength > 0) {
            std::memcpy(extra.data() + op.familyLength, text.data(), op.textLength);
        }
        m_displayList.PushOpWithData(DrawOpType::DrawText, op, extra.data(), totalExtra);
    }

    /// <summary>
    /// 繪製多行排版 TextLayout。
    /// </summary>
    /// <param name="layout">TextLayout 物件引用。</param>
    /// <param name="origin">文字基準原點。</param>
    /// <param name="color">文字填色。</param>
    void DrawTextLayout(const text::TextLayout& layout, core::Point origin, core::Color color) {
        if (layout.GetLineCount() == 0 || color.a == 0) return;
        m_displayList.RetainLayout(layout);
        DrawTextLayoutOp op;
        op.origin = core::Point(origin.x + m_currentOffset.x, origin.y + m_currentOffset.y);
        op.color = color;
        op.nativeLayoutHandle = layout.GetNativeHandle();
        m_displayList.PushOp(DrawOpType::DrawTextLayout, op);
    }

    /// <summary>
    /// 繪製橢圓邊框。
    /// </summary>
    /// <param name="rect">外接矩形邊界。</param>
    /// <param name="color">邊框色彩。</param>
    /// <param name="strokeWidth">邊框粗細。</param>
    void DrawEllipse(const core::Rect& rect, core::Color color, float strokeWidth = 1.0f) {
        if (rect.IsEmpty() || color.a == 0 || strokeWidth <= 0.0f) return;
        core::Rect offsetRect(rect.x + m_currentOffset.x, rect.y + m_currentOffset.y, rect.width, rect.height);
        DrawEllipseOp op{ offsetRect, color, strokeWidth };
        m_displayList.PushOp(DrawOpType::DrawEllipse, op);
    }

    /// <summary>
    /// 填滿橢圓內部。
    /// </summary>
    /// <param name="rect">外接矩形邊界。</param>
    /// <param name="color">填滿色彩。</param>
    void FillEllipse(const core::Rect& rect, core::Color color) {
        if (rect.IsEmpty() || color.a == 0) return;
        core::Rect offsetRect(rect.x + m_currentOffset.x, rect.y + m_currentOffset.y, rect.width, rect.height);
        FillEllipseOp op{ offsetRect, color };
        m_displayList.PushOp(DrawOpType::FillEllipse, op);
    }

    /// <summary>
    /// 描繪幾何路徑輪廓。
    /// </summary>
    /// <param name="pathPtr">幾何路徑指針。</param>
    /// <param name="color">輪廓色彩。</param>
    /// <param name="strokeWidth">描邊粗細。</param>
    void DrawPath(const void* pathPtr, core::Color color, float strokeWidth = 1.0f) {
        if (!pathPtr || color.a == 0 || strokeWidth <= 0.0f) return;
        const auto* path = static_cast<const graphics::GraphicsPath*>(pathPtr);
        auto points = path->Flatten(0.25f);
        if (points.size() < 2) return;

        DrawPathOp op;
        op.color = color;
        op.strokeWidth = strokeWidth;
        op.pointCount = static_cast<uint32_t>(points.size());
        m_displayList.PushOpWithData(DrawOpType::DrawPath, op, points.data(), points.size() * sizeof(core::Point));
    }

    /// <summary>
    /// 填滿幾何路徑內部 (支援畫刷與退回純色)。
    /// </summary>
    /// <param name="pathPtr">幾何路徑指針。</param>
    /// <param name="brushPtr">畫刷指針 (如 LinearGradientBrush)。</param>
    /// <param name="fallbackColor">備用填滿色彩。</param>
    /// <param name="fillMode">填色模式。</param>
    void FillPath(const void* pathPtr, const void* brushPtr, core::Color fallbackColor, uint8_t fillMode = 0) {
        if (!pathPtr) return;
        const auto* path = static_cast<const graphics::GraphicsPath*>(pathPtr);
        auto points = path->Flatten(0.25f);
        if (points.size() < 3) return;

        FillPathOp op;
        op.color = fallbackColor;
        op.fillMode = fillMode;
        op.hasGradient = 0;
        op.reserved = 0;
        op.pointCount = static_cast<uint32_t>(points.size());
        op.bounds = path->GetBounds();

        if (brushPtr) {
            const auto* brush = static_cast<const graphics::Brush*>(brushPtr);
            if (brush->GetType() == graphics::BrushType::LinearGradient) {
                const auto* grad = static_cast<const graphics::LinearGradientBrush*>(brush);
                op.hasGradient = 1;
                op.gradStart = grad->GetStartPoint();
                op.gradEnd = grad->GetEndPoint();
                op.gradStartColor = grad->GetStartColor();
                op.gradEndColor = grad->GetEndColor();
            }
        }

        m_displayList.PushOpWithData(DrawOpType::FillPath, op, points.data(), points.size() * sizeof(core::Point));
    }

    /// <summary>
    /// 填滿幾何路徑內部。
    /// </summary>
    /// <param name="pathPtr">幾何路徑指針。</param>
    /// <param name="color">填滿色彩。</param>
    /// <param name="fillMode">填色模式。</param>
    void FillPath(const void* pathPtr, core::Color color, uint8_t fillMode = 0) {
        FillPath(pathPtr, nullptr, color, fillMode);
    }

    /// <summary>
    /// 繪製點陣影像。
    /// </summary>
    /// <param name="bitmapPtr">點陣影像指針。</param>
    /// <param name="destRect">目標繪製矩形。</param>
    /// <param name="srcRect">來源影像裁切矩形。</param>
    /// <param name="opacity">不透明度 (0.0f ~ 1.0f)。</param>
    void DrawImage(const void* bitmapPtr, const core::Rect& destRect, const core::Rect& srcRect, float opacity = 1.0f) {
        if (!bitmapPtr || destRect.IsEmpty() || opacity <= 0.0f) return;
        core::Rect offsetDest(destRect.x + m_currentOffset.x, destRect.y + m_currentOffset.y, destRect.width, destRect.height);
        DrawImageOp op{ offsetDest, srcRect, bitmapPtr, opacity };
        m_displayList.PushOp(DrawOpType::DrawImage, op);
    }

    /// <summary>
    /// 壓入局部剪裁矩形 (Clip Rect)。
    /// </summary>
    /// <param name="rect">剪裁矩形。</param>
    void PushClip(const core::Rect& rect) {
        core::Rect offsetRect(rect.x + m_currentOffset.x, rect.y + m_currentOffset.y, rect.width, rect.height);
        PushClipRectOp op{ offsetRect };
        m_displayList.PushOp(DrawOpType::PushClipRect, op);
    }

    /// <summary>
    /// 彈出局部剪裁矩形。
    /// </summary>
    void PopClip() {
        m_displayList.PushHeaderOnly(DrawOpType::PopClip);
    }

    /// <summary>
    /// 壓入透明度 (0.0f ~ 1.0f)。
    /// </summary>
    /// <param name="opacity">不透明度值。</param>
    void PushOpacity(float opacity) {
        PushOpacityOp op{ opacity };
        m_displayList.PushOp(DrawOpType::PushOpacity, op);
    }

    /// <summary>
    /// 彈出透明度。
    /// </summary>
    void PopOpacity() {
        m_displayList.PushHeaderOnly(DrawOpType::PopOpacity);
    }

    /// <summary>
    /// 壓入局部坐標偏移量 (Offset Translation)。
    /// </summary>
    /// <param name="offset">坐標偏移向量。</param>
    void PushOffset(core::Point offset) {
        m_offsetStack.push_back(offset);
        m_currentOffset.x += offset.x;
        m_currentOffset.y += offset.y;
    }

    /// <summary>
    /// 彈出局部坐標偏移量。
    /// </summary>
    void PopOffset() {
        if (!m_offsetStack.empty()) {
            m_currentOffset.x -= m_offsetStack.back().x;
            m_currentOffset.y -= m_offsetStack.back().y;
            m_offsetStack.pop_back();
        }
    }

    /// <summary>
    /// 取得當前累積坐標偏移量。
    /// </summary>
    /// <returns>坐標偏移向量。</returns>
    COMPAT_NODISCARD core::Point GetCurrentOffset() const noexcept {
        return m_currentOffset;
    }

    /// <summary>
    /// RAII 坐標偏移自動彈出守衛
    /// </summary>
    class AutoOffset {
    public:
        AutoOffset(DrawingContext& dc, core::Point offset) : m_dc(dc) {
            m_dc.PushOffset(offset);
        }
        ~AutoOffset() {
            m_dc.PopOffset();
        }
    private:
        DrawingContext& m_dc;
    };

    /// <summary>
    /// RAII 剪裁自動彈出守衛
    /// </summary>
    class AutoClip {
    public:
        AutoClip(DrawingContext& dc, const core::Rect& rect) : m_dc(dc) {
            m_dc.PushClip(rect);
        }
        ~AutoClip() {
            m_dc.PopClip();
        }
    private:
        DrawingContext& m_dc;
    };

    /// <summary>
    /// RAII 透明度自動彈出守衛
    /// </summary>
    class AutoOpacity {
    public:
        AutoOpacity(DrawingContext& dc, float opacity) : m_dc(dc) {
            m_dc.PushOpacity(opacity);
        }
        ~AutoOpacity() {
            m_dc.PopOpacity();
        }
    private:
        DrawingContext& m_dc;
    };

private:
    DisplayList&              m_displayList;
    core::Point               m_currentOffset{ 0.0f, 0.0f };
    std::vector<core::Point>  m_offsetStack;
};

} // namespace rendering
} // namespace cppui
