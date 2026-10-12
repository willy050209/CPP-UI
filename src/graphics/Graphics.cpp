/**
 * @file Graphics.cpp
 * @brief GDI+ 風格統一繪圖排程與畫布控制中樞類別實作，具備現代 Pimpl 編譯防火牆
 */

#include "ui/graphics/Graphics.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"
#include "ui/rendering/backend/IRenderContext.hpp"
#include "ui/text/TextLayoutEngine.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace cppui {
namespace graphics {

namespace {

constexpr float PI = 3.14159265358979323846f;

core::Color GetBrushColor(const Brush& brush) {
    if (brush.GetType() == BrushType::SolidColor) {
        return static_cast<const SolidBrush&>(brush).GetColor();
    }
    if (brush.GetType() == BrushType::LinearGradient) {
        return static_cast<const LinearGradientBrush&>(brush).GetStartColor();
    }
    return core::Color(0, 0, 0, 255);
}

class SoftwareBitmapTarget : public rendering::IRenderTarget {
public:
    explicit SoftwareBitmapTarget(Bitmap& bmp) : m_bmp(bmp) {}

    uint32_t GetWidth() const noexcept override { return m_bmp.GetWidth(); }
    uint32_t GetHeight() const noexcept override { return m_bmp.GetHeight(); }

    void Clear(core::Color color) override {
        m_bmp.Clear(color);
    }

    void FillRect(const core::Rect& rect, core::Color color) override {
        uint32_t stride = m_bmp.GetWidth();
        uint32_t* pixels = m_bmp.GetPixelSpanMut().data();
        uint32_t val = color.ToArgb();

        int32_t x0 = std::max(0, static_cast<int32_t>(rect.Left()));
        int32_t y0 = std::max(0, static_cast<int32_t>(rect.Top()));
        int32_t x1 = std::min(static_cast<int32_t>(m_bmp.GetWidth()), static_cast<int32_t>(rect.Right()));
        int32_t y1 = std::min(static_cast<int32_t>(m_bmp.GetHeight()), static_cast<int32_t>(rect.Bottom()));

        for (int32_t y = y0; y < y1; ++y) {
            for (int32_t x = x0; x < x1; ++x) {
                pixels[static_cast<size_t>(y) * stride + x] = val;
            }
        }
    }

    uint32_t* GetPixelBuffer() noexcept override {
        return m_bmp.GetPixelSpanMut().data();
    }

    void* GetSkCanvas() noexcept override {
        return nullptr;
    }

private:
    Bitmap& m_bmp;
};

struct InternalState {
    float tx = 0.0f;
    float ty = 0.0f;
    float sx = 1.0f;
    float sy = 1.0f;
    float rot = 0.0f;
    core::Rect clipRect;
    SmoothingMode smoothing = SmoothingMode::Default;
    InterpolationMode interpolation = InterpolationMode::Default;
};

} // namespace

struct Graphics::Impl {
    rendering::DrawingContext* dc = nullptr;
    std::shared_ptr<Bitmap> image;
    std::unique_ptr<rendering::DisplayList> ownedList;
    std::unique_ptr<rendering::DrawingContext> ownedDC;
    InternalState currentState;
    std::vector<std::pair<uint32_t, InternalState>> stateStack;
    uint32_t nextStateId = 1;

    core::Point TransformPoint(core::Point p) const {
        float sx = p.x * currentState.sx;
        float sy = p.y * currentState.sy;
        if (currentState.rot != 0.0f) {
            float rad = currentState.rot * PI / 180.0f;
            float cosA = std::cos(rad);
            float sinA = std::sin(rad);
            float rx = sx * cosA - sy * sinA;
            float ry = sx * sinA + sy * cosA;
            return core::Point(rx + currentState.tx, ry + currentState.ty);
        }
        return core::Point(sx + currentState.tx, sy + currentState.ty);
    }

    core::Rect TransformRect(const core::Rect& r) const {
        core::Point p = TransformPoint(core::Point(r.x, r.y));
        return core::Rect(p.x, p.y, r.width * currentState.sx, r.height * currentState.sy);
    }

    void Flush() {
        if (image && ownedList && ownedList->GetOpCount() > 0) {
            rendering::DisplayListRenderer renderer;
            SoftwareBitmapTarget target(*image);
            renderer.Render(*ownedList, &target);
            ownedList->Clear();
        }
    }
};

Graphics Graphics::FromDrawingContext(rendering::DrawingContext& dc) {
    return Graphics(dc);
}

Graphics Graphics::FromImage(std::shared_ptr<Bitmap> image) {
    if (!image) {
        throw std::invalid_argument("Target image cannot be null.");
    }
    return Graphics(image);
}

Graphics::Graphics(rendering::DrawingContext& dc)
    : m_impl(std::make_unique<Impl>()) {
    m_impl->dc = &dc;
    m_impl->currentState.clipRect = core::Rect(0.0f, 0.0f, 100000.0f, 100000.0f);
}

Graphics::Graphics(std::shared_ptr<Bitmap> image)
    : m_impl(std::make_unique<Impl>()) {
    if (!image) {
        throw std::invalid_argument("Target image cannot be null.");
    }
    m_impl->image = image;
    m_impl->ownedList = std::make_unique<rendering::DisplayList>();
    m_impl->ownedDC   = std::make_unique<rendering::DrawingContext>(*m_impl->ownedList);
    m_impl->dc        = m_impl->ownedDC.get();
    m_impl->currentState.clipRect = core::Rect(0.0f, 0.0f, static_cast<float>(image->GetWidth()), static_cast<float>(image->GetHeight()));
}

Graphics::Graphics(Graphics&& other) noexcept = default;

Graphics& Graphics::operator=(Graphics&& other) noexcept {
    if (this != &other) {
        Flush();
        m_impl = std::move(other.m_impl);
    }
    return *this;
}

Graphics::~Graphics() {
    Flush();
}

uint32_t Graphics::Save() {
    if (!m_impl) return 0;
    uint32_t id = m_impl->nextStateId++;
    m_impl->stateStack.emplace_back(id, m_impl->currentState);
    return id;
}

void Graphics::Restore(uint32_t stateId) {
    if (!m_impl) return;
    while (!m_impl->stateStack.empty()) {
        auto top = m_impl->stateStack.back();
        m_impl->stateStack.pop_back();
        if (top.first == stateId) {
            m_impl->currentState = top.second;
            break;
        }
    }
}

void Graphics::TranslateTransform(float dx, float dy) {
    if (!m_impl) return;
    m_impl->currentState.tx += dx;
    m_impl->currentState.ty += dy;
}

void Graphics::ScaleTransform(float sx, float sy) {
    if (!m_impl) return;
    m_impl->currentState.sx *= sx;
    m_impl->currentState.sy *= sy;
}

void Graphics::RotateTransform(float angleDegrees) {
    if (!m_impl) return;
    m_impl->currentState.rot += angleDegrees;
}

void Graphics::ResetTransform() {
    if (!m_impl) return;
    m_impl->currentState.tx = 0.0f;
    m_impl->currentState.ty = 0.0f;
    m_impl->currentState.sx = 1.0f;
    m_impl->currentState.sy = 1.0f;
    m_impl->currentState.rot = 0.0f;
}

void Graphics::SetClip(const core::Rect& rect) {
    if (!m_impl) return;
    m_impl->currentState.clipRect = m_impl->TransformRect(rect);
    if (m_impl->dc) {
        m_impl->dc->PushClip(m_impl->currentState.clipRect);
    }
}

void Graphics::SetClip(const GraphicsPath& path) {
    SetClip(path.GetBounds());
}

void Graphics::ResetClip() {
    if (!m_impl) return;
    if (m_impl->image) {
        m_impl->currentState.clipRect = core::Rect(0.0f, 0.0f, static_cast<float>(m_impl->image->GetWidth()), static_cast<float>(m_impl->image->GetHeight()));
    } else {
        m_impl->currentState.clipRect = core::Rect(0.0f, 0.0f, 100000.0f, 100000.0f);
    }
    if (m_impl->dc) {
        m_impl->dc->PopClip();
    }
}

core::Rect Graphics::GetClipBounds() const {
    return m_impl ? m_impl->currentState.clipRect : core::Rect{};
}

void Graphics::SetSmoothingMode(SmoothingMode mode) {
    if (!m_impl) return;
    m_impl->currentState.smoothing = mode;
}

SmoothingMode Graphics::GetSmoothingMode() const {
    return m_impl ? m_impl->currentState.smoothing : SmoothingMode::Default;
}

void Graphics::SetInterpolationMode(InterpolationMode mode) {
    if (!m_impl) return;
    m_impl->currentState.interpolation = mode;
}

InterpolationMode Graphics::GetInterpolationMode() const {
    return m_impl ? m_impl->currentState.interpolation : InterpolationMode::Default;
}

void Graphics::Clear(core::Color color) {
    if (!m_impl || !m_impl->dc) return;
    m_impl->dc->DrawRectangle(m_impl->currentState.clipRect, color);
}

void Graphics::DrawLine(const Pen& pen, core::Point pt1, core::Point pt2) {
    if (!m_impl || !m_impl->dc) return;
    m_impl->dc->DrawLine(m_impl->TransformPoint(pt1), m_impl->TransformPoint(pt2), pen.GetColor(), pen.GetWidth());
}

void Graphics::DrawLine(const Pen& pen, float x1, float y1, float x2, float y2) {
    DrawLine(pen, core::Point(x1, y1), core::Point(x2, y2));
}

void Graphics::DrawLines(const Pen& pen, compat::span<const core::Point> points) {
    if (points.size() < 2) return;
    for (size_t i = 0; i + 1 < points.size(); ++i) {
        DrawLine(pen, points[i], points[i + 1]);
    }
}

void Graphics::DrawRectangle(const Pen& pen, const core::Rect& rect) {
    if (rect.IsEmpty() || !m_impl || !m_impl->dc) return;
    core::Point p1(rect.Left(), rect.Top());
    core::Point p2(rect.Right(), rect.Top());
    core::Point p3(rect.Right(), rect.Bottom());
    core::Point p4(rect.Left(), rect.Bottom());

    DrawLine(pen, p1, p2);
    DrawLine(pen, p2, p3);
    DrawLine(pen, p3, p4);
    DrawLine(pen, p4, p1);
}

void Graphics::DrawRectangle(const Pen& pen, float x, float y, float width, float height) {
    DrawRectangle(pen, core::Rect(x, y, width, height));
}

void Graphics::DrawRectangles(const Pen& pen, compat::span<const core::Rect> rects) {
    for (const auto& r : rects) {
        DrawRectangle(pen, r);
    }
}

void Graphics::FillRectangle(const Brush& brush, const core::Rect& rect) {
    if (rect.IsEmpty() || !m_impl || !m_impl->dc) return;
    m_impl->dc->DrawRectangle(m_impl->TransformRect(rect), GetBrushColor(brush));
}

void Graphics::FillRectangle(const Brush& brush, float x, float y, float width, float height) {
    FillRectangle(brush, core::Rect(x, y, width, height));
}

void Graphics::FillRectangles(const Brush& brush, compat::span<const core::Rect> rects) {
    for (const auto& r : rects) {
        FillRectangle(brush, r);
    }
}

void Graphics::DrawRoundedRectangle(const Pen& pen, const core::Rect& rect, float radius) {
    GraphicsPath path;
    path.AddRoundedRectangle(rect, radius);
    DrawPath(pen, path);
}

void Graphics::FillRoundedRectangle(const Brush& brush, const core::Rect& rect, float radius) {
    if (!m_impl || !m_impl->dc || rect.IsEmpty()) return;
    m_impl->dc->DrawRoundedRectangle(m_impl->TransformRect(rect), radius, GetBrushColor(brush));
}

void Graphics::DrawEllipse(const Pen& pen, const core::Rect& rect) {
    if (!m_impl || !m_impl->dc || rect.IsEmpty()) return;
    m_impl->dc->DrawEllipse(m_impl->TransformRect(rect), pen.GetColor(), pen.GetWidth());
}

void Graphics::DrawEllipse(const Pen& pen, float x, float y, float width, float height) {
    DrawEllipse(pen, core::Rect(x, y, width, height));
}

void Graphics::FillEllipse(const Brush& brush, const core::Rect& rect) {
    if (!m_impl || !m_impl->dc || rect.IsEmpty()) return;
    m_impl->dc->FillEllipse(m_impl->TransformRect(rect), GetBrushColor(brush));
}

void Graphics::FillEllipse(const Brush& brush, float x, float y, float width, float height) {
    FillEllipse(brush, core::Rect(x, y, width, height));
}

void Graphics::DrawArc(const Pen& pen, const core::Rect& rect, float startAngle, float sweepAngle) {
    GraphicsPath path;
    path.AddArc(rect, startAngle, sweepAngle);
    DrawPath(pen, path);
}

void Graphics::DrawPie(const Pen& pen, const core::Rect& rect, float startAngle, float sweepAngle) {
    GraphicsPath path;
    float cx = rect.x + rect.width * 0.5f;
    float cy = rect.y + rect.height * 0.5f;
    path.AddLine(core::Point(cx, cy), core::Point(cx + rect.width * 0.5f * std::cos(startAngle * PI / 180.0f),
                                                  cy + rect.height * 0.5f * std::sin(startAngle * PI / 180.0f)));
    path.AddArc(rect, startAngle, sweepAngle);
    path.CloseFigure();
    DrawPath(pen, path);
}

void Graphics::FillPie(const Brush& brush, const core::Rect& rect, float startAngle, float sweepAngle) {
    GraphicsPath path;
    float cx = rect.x + rect.width * 0.5f;
    float cy = rect.y + rect.height * 0.5f;
    path.AddLine(core::Point(cx, cy), core::Point(cx + rect.width * 0.5f * std::cos(startAngle * PI / 180.0f),
                                                  cy + rect.height * 0.5f * std::sin(startAngle * PI / 180.0f)));
    path.AddArc(rect, startAngle, sweepAngle);
    path.CloseFigure();
    FillPath(brush, path);
}

void Graphics::DrawPolygon(const Pen& pen, compat::span<const core::Point> points) {
    GraphicsPath path;
    path.AddPolygon(points);
    DrawPath(pen, path);
}

void Graphics::FillPolygon(const Brush& brush, compat::span<const core::Point> points, FillMode fillMode) {
    GraphicsPath path(fillMode);
    path.AddPolygon(points);
    FillPath(brush, path);
}

void Graphics::DrawBezier(const Pen& pen, core::Point pt1, core::Point pt2, core::Point pt3, core::Point pt4) {
    GraphicsPath path;
    path.AddBezier(pt1, pt2, pt3, pt4);
    DrawPath(pen, path);
}

void Graphics::DrawBeziers(const Pen& pen, compat::span<const core::Point> points) {
    if (points.size() < 4) return;
    GraphicsPath path;
    for (size_t i = 0; i + 3 < points.size(); i += 3) {
        path.AddBezier(points[i], points[i + 1], points[i + 2], points[i + 3]);
    }
    DrawPath(pen, path);
}

void Graphics::DrawCurve(const Pen& pen, compat::span<const core::Point> points) {
    GraphicsPath path;
    path.AddCurve(points);
    DrawPath(pen, path);
}

void Graphics::DrawPath(const Pen& pen, const GraphicsPath& path) {
    if (!m_impl || !m_impl->dc) return;
    GraphicsPath transformedPath = path.Clone();
    if (m_impl->currentState.tx != 0.0f || m_impl->currentState.ty != 0.0f || m_impl->currentState.sx != 1.0f || m_impl->currentState.sy != 1.0f) {
        transformedPath.Transform(m_impl->currentState.sx, 0.0f, 0.0f, m_impl->currentState.sy, m_impl->currentState.tx, m_impl->currentState.ty);
    }
    m_impl->dc->DrawPath(&transformedPath, pen.GetColor(), pen.GetWidth());
}

void Graphics::FillPath(const Brush& brush, const GraphicsPath& path) {
    if (!m_impl || !m_impl->dc) return;
    GraphicsPath transformedPath = path.Clone();
    if (m_impl->currentState.tx != 0.0f || m_impl->currentState.ty != 0.0f || m_impl->currentState.sx != 1.0f || m_impl->currentState.sy != 1.0f) {
        transformedPath.Transform(m_impl->currentState.sx, 0.0f, 0.0f, m_impl->currentState.sy, m_impl->currentState.tx, m_impl->currentState.ty);
    }
    m_impl->dc->FillPath(&transformedPath, &brush, GetBrushColor(brush), static_cast<uint8_t>(path.GetFillMode()));
}

void Graphics::DrawImage(std::shared_ptr<Bitmap> image, const core::Rect& destRect) {
    if (!image) return;
    core::Rect srcRect(0.0f, 0.0f, static_cast<float>(image->GetWidth()), static_cast<float>(image->GetHeight()));
    DrawImage(image, destRect, srcRect);
}

void Graphics::DrawImage(std::shared_ptr<Bitmap> image, const core::Rect& destRect, const core::Rect& srcRect) {
    if (!image || !m_impl || !m_impl->dc) return;
    m_impl->dc->DrawImage(image.get(), m_impl->TransformRect(destRect), srcRect);
}

void Graphics::DrawImage(std::shared_ptr<Bitmap> image, float x, float y) {
    if (!image) return;
    DrawImage(image, core::Rect(x, y, static_cast<float>(image->GetWidth()), static_cast<float>(image->GetHeight())));
}

void Graphics::DrawImage(std::shared_ptr<Bitmap> image, float x, float y, float width, float height) {
    DrawImage(image, core::Rect(x, y, width, height));
}

void Graphics::DrawImageUnscaled(std::shared_ptr<Bitmap> image, core::Point origin) {
    DrawImage(image, origin.x, origin.y);
}

void Graphics::DrawString(compat::string_view text, const text::FontDescriptor& font, const Brush& brush, core::Point origin) {
    if (!m_impl || !m_impl->dc || text.empty()) return;
    m_impl->dc->DrawText(text, m_impl->TransformPoint(origin), font, GetBrushColor(brush));
}

void Graphics::DrawString(compat::string_view text, const text::FontDescriptor& font, const Brush& brush, const core::Rect& layoutRect) {
    DrawString(text, font, brush, core::Point(layoutRect.x, layoutRect.y));
}

core::Size Graphics::MeasureString(compat::string_view text, const text::FontDescriptor& font) {
    return text::TextLayoutEngine::Get().MeasureText(text, font, 10000.0f);
}

void Graphics::Flush() {
    if (m_impl) {
        m_impl->Flush();
    }
}

} // namespace graphics
} // namespace cppui
