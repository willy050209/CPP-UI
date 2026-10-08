/**
 * @file Graphics.cpp
 * @brief GDI+ 風格統一繪圖排程與畫布控制中樞類別實作
 */

#include "ui/graphics/Graphics.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"
#include "ui/rendering/backend/IRenderContext.hpp"
#include "ui/text/TextLayoutEngine.hpp"
#include <cmath>
#include <stdexcept>
#include <algorithm>

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

} // namespace

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
    : m_dc(&dc) {
    m_currentState.clipRect = core::Rect(0.0f, 0.0f, 100000.0f, 100000.0f);
}

Graphics::Graphics(std::shared_ptr<Bitmap> image)
    : m_image(image) {
    if (!image) {
        throw std::invalid_argument("Target image cannot be null.");
    }
    m_ownedList = std::make_unique<rendering::DisplayList>();
    m_ownedDC   = std::make_unique<rendering::DrawingContext>(*m_ownedList);
    m_dc        = m_ownedDC.get();
    m_currentState.clipRect = core::Rect(0.0f, 0.0f, static_cast<float>(image->GetWidth()), static_cast<float>(image->GetHeight()));
}

Graphics::Graphics(Graphics&& other) noexcept
    : m_dc(other.m_dc)
    , m_image(std::move(other.m_image))
    , m_ownedList(std::move(other.m_ownedList))
    , m_ownedDC(std::move(other.m_ownedDC))
    , m_currentState(other.m_currentState)
    , m_stateStack(std::move(other.m_stateStack))
    , m_nextStateId(other.m_nextStateId) {
    other.m_dc = nullptr;
}

Graphics& Graphics::operator=(Graphics&& other) noexcept {
    if (this != &other) {
        Flush();
        m_dc           = other.m_dc;
        m_image        = std::move(other.m_image);
        m_ownedList    = std::move(other.m_ownedList);
        m_ownedDC      = std::move(other.m_ownedDC);
        m_currentState = other.m_currentState;
        m_stateStack   = std::move(other.m_stateStack);
        m_nextStateId  = other.m_nextStateId;
        other.m_dc     = nullptr;
    }
    return *this;
}

Graphics::~Graphics() {
    Flush();
}

core::Point Graphics::TransformPoint(core::Point p) const {
    float sx = p.x * m_currentState.sx;
    float sy = p.y * m_currentState.sy;
    if (m_currentState.rot != 0.0f) {
        float rad = m_currentState.rot * PI / 180.0f;
        float cosA = std::cos(rad);
        float sinA = std::sin(rad);
        float rx = sx * cosA - sy * sinA;
        float ry = sx * sinA + sy * cosA;
        return core::Point(rx + m_currentState.tx, ry + m_currentState.ty);
    }
    return core::Point(sx + m_currentState.tx, sy + m_currentState.ty);
}

core::Rect Graphics::TransformRect(const core::Rect& r) const {
    core::Point p = TransformPoint(core::Point(r.x, r.y));
    return core::Rect(p.x, p.y, r.width * m_currentState.sx, r.height * m_currentState.sy);
}

uint32_t Graphics::Save() {
    uint32_t id = m_nextStateId++;
    m_stateStack.emplace_back(id, m_currentState);
    return id;
}

void Graphics::Restore(uint32_t stateId) {
    while (!m_stateStack.empty()) {
        auto top = m_stateStack.back();
        m_stateStack.pop_back();
        if (top.first == stateId) {
            m_currentState = top.second;
            break;
        }
    }
}

void Graphics::TranslateTransform(float dx, float dy) {
    m_currentState.tx += dx;
    m_currentState.ty += dy;
}

void Graphics::ScaleTransform(float sx, float sy) {
    m_currentState.sx *= sx;
    m_currentState.sy *= sy;
}

void Graphics::RotateTransform(float angleDegrees) {
    m_currentState.rot += angleDegrees;
}

void Graphics::ResetTransform() {
    m_currentState.tx = 0.0f;
    m_currentState.ty = 0.0f;
    m_currentState.sx = 1.0f;
    m_currentState.sy = 1.0f;
    m_currentState.rot = 0.0f;
}

void Graphics::SetClip(const core::Rect& rect) {
    m_currentState.clipRect = TransformRect(rect);
    if (m_dc) {
        m_dc->PushClip(m_currentState.clipRect);
    }
}

void Graphics::SetClip(const GraphicsPath& path) {
    SetClip(path.GetBounds());
}

void Graphics::ResetClip() {
    if (m_image) {
        m_currentState.clipRect = core::Rect(0.0f, 0.0f, static_cast<float>(m_image->GetWidth()), static_cast<float>(m_image->GetHeight()));
    } else {
        m_currentState.clipRect = core::Rect(0.0f, 0.0f, 100000.0f, 100000.0f);
    }
    if (m_dc) {
        m_dc->PopClip();
    }
}

core::Rect Graphics::GetClipBounds() const {
    return m_currentState.clipRect;
}

void Graphics::SetSmoothingMode(SmoothingMode mode) {
    m_currentState.smoothing = mode;
}

SmoothingMode Graphics::GetSmoothingMode() const {
    return m_currentState.smoothing;
}

void Graphics::SetInterpolationMode(InterpolationMode mode) {
    m_currentState.interpolation = mode;
}

InterpolationMode Graphics::GetInterpolationMode() const {
    return m_currentState.interpolation;
}

void Graphics::Clear(core::Color color) {
    if (!m_dc) return;
    m_dc->DrawRectangle(m_currentState.clipRect, color);
}

void Graphics::DrawLine(const Pen& pen, core::Point pt1, core::Point pt2) {
    if (!m_dc) return;
    m_dc->DrawLine(TransformPoint(pt1), TransformPoint(pt2), pen.GetColor(), pen.GetWidth());
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
    if (rect.IsEmpty() || !m_dc) return;
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
    if (rect.IsEmpty() || !m_dc) return;
    m_dc->DrawRectangle(TransformRect(rect), GetBrushColor(brush));
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
    if (!m_dc || rect.IsEmpty()) return;
    m_dc->DrawRoundedRectangle(TransformRect(rect), radius, GetBrushColor(brush));
}

void Graphics::DrawEllipse(const Pen& pen, const core::Rect& rect) {
    if (!m_dc || rect.IsEmpty()) return;
    m_dc->DrawEllipse(TransformRect(rect), pen.GetColor(), pen.GetWidth());
}

void Graphics::DrawEllipse(const Pen& pen, float x, float y, float width, float height) {
    DrawEllipse(pen, core::Rect(x, y, width, height));
}

void Graphics::FillEllipse(const Brush& brush, const core::Rect& rect) {
    if (!m_dc || rect.IsEmpty()) return;
    m_dc->FillEllipse(TransformRect(rect), GetBrushColor(brush));
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
    if (!m_dc) return;
    GraphicsPath transformedPath = path.Clone();
    if (m_currentState.tx != 0.0f || m_currentState.ty != 0.0f || m_currentState.sx != 1.0f || m_currentState.sy != 1.0f) {
        transformedPath.Transform(m_currentState.sx, 0.0f, 0.0f, m_currentState.sy, m_currentState.tx, m_currentState.ty);
    }
    m_dc->DrawPath(&transformedPath, pen.GetColor(), pen.GetWidth());
}

void Graphics::FillPath(const Brush& brush, const GraphicsPath& path) {
    if (!m_dc) return;
    GraphicsPath transformedPath = path.Clone();
    if (m_currentState.tx != 0.0f || m_currentState.ty != 0.0f || m_currentState.sx != 1.0f || m_currentState.sy != 1.0f) {
        transformedPath.Transform(m_currentState.sx, 0.0f, 0.0f, m_currentState.sy, m_currentState.tx, m_currentState.ty);
    }
    m_dc->FillPath(&transformedPath, &brush, GetBrushColor(brush), static_cast<uint8_t>(path.GetFillMode()));
}

void Graphics::DrawImage(std::shared_ptr<Bitmap> image, const core::Rect& destRect) {
    if (!image) return;
    core::Rect srcRect(0.0f, 0.0f, static_cast<float>(image->GetWidth()), static_cast<float>(image->GetHeight()));
    DrawImage(image, destRect, srcRect);
}

void Graphics::DrawImage(std::shared_ptr<Bitmap> image, const core::Rect& destRect, const core::Rect& srcRect) {
    if (!image || !m_dc) return;
    m_dc->DrawImage(image.get(), TransformRect(destRect), srcRect);
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
    if (!m_dc || text.empty()) return;
    m_dc->DrawText(text, TransformPoint(origin), font, GetBrushColor(brush));
}

void Graphics::DrawString(compat::string_view text, const text::FontDescriptor& font, const Brush& brush, const core::Rect& layoutRect) {
    DrawString(text, font, brush, core::Point(layoutRect.x, layoutRect.y));
}

core::Size Graphics::MeasureString(compat::string_view text, const text::FontDescriptor& font) {
    return text::TextLayoutEngine::Get().MeasureText(text, font, 10000.0f);
}

void Graphics::Flush() {
    if (m_image && m_ownedList && m_ownedList->GetOpCount() > 0) {
        rendering::DisplayListRenderer renderer;
        SoftwareBitmapTarget target(*m_image);
        renderer.Render(*m_ownedList, &target);
        m_ownedList->Clear();
    }
}

} // namespace graphics
} // namespace cppui
