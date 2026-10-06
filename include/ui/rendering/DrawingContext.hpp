#pragma once

#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/text/Font.hpp"
#include "ui/text/TextLayout.hpp"

namespace cppui {
namespace rendering {

/// <summary>
/// 公開畫布錄製抽象介面 (DrawingContext)
/// UI 控制項在此產生純量繪製命令，將操作序列化寫入 DisplayList，不直接觸發 GPU/光柵化。
/// </summary>
class DrawingContext {
public:
    explicit DrawingContext(DisplayList& displayList) noexcept
        : m_displayList(displayList) {}

    ~DrawingContext() = default;

    /// <summary>
    /// 繪製填色矩形
    /// </summary>
    void DrawRectangle(const core::Rect& rect, core::Color color) {
        if (rect.IsEmpty() || color.a == 0) return;
        core::Rect offsetRect(rect.x + m_currentOffset.x, rect.y + m_currentOffset.y, rect.width, rect.height);
        DrawRectOp op{ offsetRect, color };
        m_displayList.PushOp(DrawOpType::DrawRectangle, op);
    }

    /// <summary>
    /// 繪製圓角矩形
    /// </summary>
    void DrawRoundedRectangle(const core::Rect& rect, float radius, core::Color color) {
        if (rect.IsEmpty() || color.a == 0) return;
        core::Rect offsetRect(rect.x + m_currentOffset.x, rect.y + m_currentOffset.y, rect.width, rect.height);
        DrawRoundedRectOp op{ offsetRect, radius, color };
        m_displayList.PushOp(DrawOpType::DrawRoundedRectangle, op);
    }

    /// <summary>
    /// 繪製線段
    /// </summary>
    void DrawLine(core::Point p1, core::Point p2, core::Color color, float strokeWidth = 1.0f) {
        if (color.a == 0 || strokeWidth <= 0.0f) return;
        core::Point op1(p1.x + m_currentOffset.x, p1.y + m_currentOffset.y);
        core::Point op2(p2.x + m_currentOffset.x, p2.y + m_currentOffset.y);
        DrawLineOp op{ op1, op2, color, strokeWidth };
        m_displayList.PushOp(DrawOpType::DrawLine, op);
    }

    /// <summary>
    /// 繪製單行純文字
    /// </summary>
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
    /// 繪製多行排版 TextLayout
    /// </summary>
    void DrawTextLayout(const text::TextLayout& layout, core::Point origin, core::Color color) {
        if (layout.GetLineCount() == 0 || color.a == 0) return;
        DrawTextLayoutOp op;
        op.origin = core::Point(origin.x + m_currentOffset.x, origin.y + m_currentOffset.y);
        op.color = color;
        op.nativeLayoutHandle = layout.GetNativeHandle();
        m_displayList.PushOp(DrawOpType::DrawTextLayout, op);
    }

    /// <summary>
    /// 壓入局部剪裁矩形 (Clip Rect)
    /// </summary>
    void PushClip(const core::Rect& rect) {
        core::Rect offsetRect(rect.x + m_currentOffset.x, rect.y + m_currentOffset.y, rect.width, rect.height);
        PushClipRectOp op{ offsetRect };
        m_displayList.PushOp(DrawOpType::PushClipRect, op);
    }

    /// <summary>
    /// 彈出局部剪裁矩形
    /// </summary>
    void PopClip() {
        m_displayList.PushHeaderOnly(DrawOpType::PopClip);
    }

    /// <summary>
    /// 壓入透明度 (0.0f ~ 1.0f)
    /// </summary>
    void PushOpacity(float opacity) {
        PushOpacityOp op{ opacity };
        m_displayList.PushOp(DrawOpType::PushOpacity, op);
    }

    /// <summary>
    /// 彈出透明度
    /// </summary>
    void PopOpacity() {
        m_displayList.PushHeaderOnly(DrawOpType::PopOpacity);
    }

    /// <summary>
    /// 壓入局部坐標偏移量 (Offset Translation)
    /// </summary>
    void PushOffset(core::Point offset) {
        m_offsetStack.push_back(offset);
        m_currentOffset.x += offset.x;
        m_currentOffset.y += offset.y;
    }

    /// <summary>
    /// 彈出局部坐標偏移量
    /// </summary>
    void PopOffset() {
        if (!m_offsetStack.empty()) {
            m_currentOffset.x -= m_offsetStack.back().x;
            m_currentOffset.y -= m_offsetStack.back().y;
            m_offsetStack.pop_back();
        }
    }

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
