/**
 * @file GraphicsPath.cpp
 * @brief GDI+ 風格幾何路徑實作，支援線段、貝茲曲線、多邊形與執行緒安全自動同步
 */

#include "ui/graphics/GraphicsPath.hpp"
#include "ui/graphics/Pen.hpp"
#include <cmath>
#include <algorithm>

namespace cppui {
namespace graphics {

namespace {

constexpr float PI = 3.14159265358979323846f;
// 標準三次貝茲曲線擬合圓弧的 magic number kappa = 4/3 * (sqrt(2) - 1)
constexpr float KAPPA = 0.5522847498307933984f;

// 點至線段最短平方距離
float DistanceSqToSegment(core::Point p, core::Point a, core::Point b) {
    float l2 = (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
    if (l2 <= 0.00001f) {
        return (p.x - a.x) * (p.x - a.x) + (p.y - a.y) * (p.y - a.y);
    }
    float t = ((p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y)) / l2;
    t = compat::clamp(t, 0.0f, 1.0f);
    core::Point proj(a.x + t * (b.x - a.x), a.y + t * (b.y - a.y));
    return (p.x - proj.x) * (p.x - proj.x) + (p.y - proj.y) * (p.y - proj.y);
}

// 遞迴平整化三次貝茲曲線
void SubdivideCubicBezier(
    core::Point p1, core::Point c1, core::Point c2, core::Point p2,
    float flatnessSq, std::vector<core::Point>& outPoints, int32_t depth = 0
) {
    if (depth > 10) {
        outPoints.push_back(p2);
        return;
    }

    // 計算控制點到弦線的垂直偏差
    float d1 = DistanceSqToSegment(c1, p1, p2);
    float d2 = DistanceSqToSegment(c2, p1, p2);

    if (d1 <= flatnessSq && d2 <= flatnessSq) {
        outPoints.push_back(p2);
        return;
    }

    // de Casteljau 切分
    core::Point p12((p1.x + c1.x) * 0.5f, (p1.y + c1.y) * 0.5f);
    core::Point p23((c1.x + c2.x) * 0.5f, (c1.y + c2.y) * 0.5f);
    core::Point p34((c2.x + p2.x) * 0.5f, (c2.y + p2.y) * 0.5f);

    core::Point p123((p12.x + p23.x) * 0.5f, (p12.y + p23.y) * 0.5f);
    core::Point p234((p23.x + p34.x) * 0.5f, (p23.y + p34.y) * 0.5f);

    core::Point p1234((p123.x + p234.x) * 0.5f, (p123.y + p234.y) * 0.5f);

    SubdivideCubicBezier(p1, p12, p123, p1234, flatnessSq, outPoints, depth + 1);
    SubdivideCubicBezier(p1234, p234, p34, p2, flatnessSq, outPoints, depth + 1);
}

} // namespace

GraphicsPath::GraphicsPath()
    : m_fillMode(FillMode::Alternate) {}

GraphicsPath::GraphicsPath(FillMode fillMode)
    : m_fillMode(fillMode) {}

GraphicsPath::GraphicsPath(compat::span<const core::Point> points, FillMode fillMode)
    : m_fillMode(fillMode) {
    AddPolygon(points);
}

GraphicsPath::GraphicsPath(const GraphicsPath& other) {
    std::shared_lock<std::shared_mutex> lock(other.m_mutex);
    m_points       = other.m_points;
    m_types        = other.m_types;
    m_fillMode     = other.m_fillMode;
    m_isFigureOpen = other.m_isFigureOpen;
}

GraphicsPath::GraphicsPath(GraphicsPath&& other) noexcept {
    std::unique_lock<std::shared_mutex> lock(other.m_mutex);
    m_points       = std::move(other.m_points);
    m_types        = std::move(other.m_types);
    m_fillMode     = other.m_fillMode;
    m_isFigureOpen = other.m_isFigureOpen;
}

GraphicsPath& GraphicsPath::operator=(const GraphicsPath& other) {
    if (this != &other) {
        std::unique_lock<std::shared_mutex> lockThis(m_mutex, std::defer_lock);
        std::shared_lock<std::shared_mutex> lockOther(other.m_mutex, std::defer_lock);
        std::lock(lockThis, lockOther);

        m_points       = other.m_points;
        m_types        = other.m_types;
        m_fillMode     = other.m_fillMode;
        m_isFigureOpen = other.m_isFigureOpen;
    }
    return *this;
}

GraphicsPath& GraphicsPath::operator=(GraphicsPath&& other) noexcept {
    if (this != &other) {
        std::unique_lock<std::shared_mutex> lockThis(m_mutex, std::defer_lock);
        std::unique_lock<std::shared_mutex> lockOther(other.m_mutex, std::defer_lock);
        std::lock(lockThis, lockOther);

        m_points       = std::move(other.m_points);
        m_types        = std::move(other.m_types);
        m_fillMode     = other.m_fillMode;
        m_isFigureOpen = other.m_isFigureOpen;
    }
    return *this;
}

void GraphicsPath::AddLine(core::Point p1, core::Point p2) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    if (!m_isFigureOpen || m_points.empty() || m_points.back() != p1) {
        m_points.push_back(p1);
        m_types.push_back(static_cast<uint8_t>(PathPointType::Start));
    }
    m_points.push_back(p2);
    m_types.push_back(static_cast<uint8_t>(PathPointType::Line));
    m_isFigureOpen = true;
}

void GraphicsPath::AddLine(float x1, float y1, float x2, float y2) {
    AddLine(core::Point(x1, y1), core::Point(x2, y2));
}

void GraphicsPath::AddLines(compat::span<const core::Point> points) {
    if (points.size() < 2) return;
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    if (!m_isFigureOpen || m_points.empty() || m_points.back() != points[0]) {
        m_points.push_back(points[0]);
        m_types.push_back(static_cast<uint8_t>(PathPointType::Start));
    }
    for (size_t i = 1; i < points.size(); ++i) {
        m_points.push_back(points[i]);
        m_types.push_back(static_cast<uint8_t>(PathPointType::Line));
    }
    m_isFigureOpen = true;
}

void GraphicsPath::AddRectangle(const core::Rect& rect) {
    if (rect.IsEmpty()) return;
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_points.push_back(core::Point(rect.Left(), rect.Top()));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Start));

    m_points.push_back(core::Point(rect.Right(), rect.Top()));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Line));

    m_points.push_back(core::Point(rect.Right(), rect.Bottom()));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Line));

    m_points.push_back(core::Point(rect.Left(), rect.Bottom()));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Line) | static_cast<uint8_t>(PathPointType::CloseSubpath));
    m_isFigureOpen = false;
}

void GraphicsPath::AddRoundedRectangle(const core::Rect& rect, float radius) {
    if (rect.IsEmpty()) return;
    float r = compat::clamp(radius, 0.0f, std::min(rect.width, rect.height) * 0.5f);
    if (r <= 0.0f) {
        AddRectangle(rect);
        return;
    }

    std::unique_lock<std::shared_mutex> lock(m_mutex);
    float k = r * KAPPA;

    // 起點在頂部邊緣右移 r 處
    m_points.push_back(core::Point(rect.Left() + r, rect.Top()));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Start));

    // 頂部直線
    m_points.push_back(core::Point(rect.Right() - r, rect.Top()));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Line));

    // 右上圓角
    m_points.push_back(core::Point(rect.Right() - r + k, rect.Top()));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(core::Point(rect.Right(), rect.Top() + r - k));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(core::Point(rect.Right(), rect.Top() + r));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));

    // 右側直線
    m_points.push_back(core::Point(rect.Right(), rect.Bottom() - r));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Line));

    // 右下圓角
    m_points.push_back(core::Point(rect.Right(), rect.Bottom() - r + k));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(core::Point(rect.Right() - r + k, rect.Bottom()));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(core::Point(rect.Right() - r, rect.Bottom()));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));

    // 底部直線
    m_points.push_back(core::Point(rect.Left() + r, rect.Bottom()));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Line));

    // 左下圓角
    m_points.push_back(core::Point(rect.Left() + r - k, rect.Bottom()));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(core::Point(rect.Left(), rect.Bottom() - r + k));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(core::Point(rect.Left(), rect.Bottom() - r));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));

    // 左側直線
    m_points.push_back(core::Point(rect.Left(), rect.Top() + r));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Line));

    // 左上圓角並閉合
    m_points.push_back(core::Point(rect.Left(), rect.Top() + r - k));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(core::Point(rect.Left() + r - k, rect.Top()));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(core::Point(rect.Left() + r, rect.Top()));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier) | static_cast<uint8_t>(PathPointType::CloseSubpath));
    m_isFigureOpen = false;
}

void GraphicsPath::AddEllipse(const core::Rect& rect) {
    if (rect.IsEmpty()) return;
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    float cx = rect.x + rect.width * 0.5f;
    float cy = rect.y + rect.height * 0.5f;
    float rx = rect.width * 0.5f;
    float ry = rect.height * 0.5f;
    float kx = rx * KAPPA;
    float ky = ry * KAPPA;

    m_points.push_back(core::Point(cx, cy - ry));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Start));

    // 第 1 象限圓弧
    m_points.push_back(core::Point(cx + kx, cy - ry));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(core::Point(cx + rx, cy - ky));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(core::Point(cx + rx, cy));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));

    // 第 4 象限圓弧
    m_points.push_back(core::Point(cx + rx, cy + ky));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(core::Point(cx + kx, cy + ry));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(core::Point(cx, cy + ry));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));

    // 第 3 象限圓弧
    m_points.push_back(core::Point(cx - kx, cy + ry));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(core::Point(cx - rx, cy + ky));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(core::Point(cx - rx, cy));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));

    // 第 2 象限圓弧並閉合
    m_points.push_back(core::Point(cx - rx, cy - ky));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(core::Point(cx - kx, cy - ry));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(core::Point(cx, cy - ry));
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier) | static_cast<uint8_t>(PathPointType::CloseSubpath));
    m_isFigureOpen = false;
}

void GraphicsPath::AddEllipse(float x, float y, float width, float height) {
    AddEllipse(core::Rect(x, y, width, height));
}

void GraphicsPath::AddArc(const core::Rect& rect, float startAngle, float sweepAngle) {
    if (rect.IsEmpty() || sweepAngle == 0.0f) return;
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    float cx = rect.x + rect.width * 0.5f;
    float cy = rect.y + rect.height * 0.5f;
    float rx = rect.width * 0.5f;
    float ry = rect.height * 0.5f;

    // 將圓弧細分為不超過 90 度的扇形進行三次貝茲逼近
    int32_t segments = std::max(1, static_cast<int32_t>(std::ceil(std::abs(sweepAngle) / 90.0f)));
    float segSweep = sweepAngle / static_cast<float>(segments);

    float currentAngle = startAngle;
    for (int32_t i = 0; i < segments; ++i) {
        float a1 = currentAngle * PI / 180.0f;
        float a2 = (currentAngle + segSweep) * PI / 180.0f;
        float halfDelta = (a2 - a1) * 0.5f;
        float k = (4.0f / 3.0f) * (1.0f - std::cos(halfDelta)) / std::sin(halfDelta);

        core::Point p1(cx + rx * std::cos(a1), cy + ry * std::sin(a1));
        core::Point p2(cx + rx * std::cos(a2), cy + ry * std::sin(a2));

        core::Point c1(p1.x - k * rx * std::sin(a1), p1.y + k * ry * std::cos(a1));
        core::Point c2(p2.x + k * rx * std::sin(a2), p2.y - k * ry * std::cos(a2));

        if (i == 0) {
            if (!m_isFigureOpen || m_points.empty() || m_points.back() != p1) {
                m_points.push_back(p1);
                m_types.push_back(static_cast<uint8_t>(PathPointType::Start));
            }
        }

        m_points.push_back(c1);
        m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
        m_points.push_back(c2);
        m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
        m_points.push_back(p2);
        m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));

        currentAngle += segSweep;
    }
    m_isFigureOpen = true;
}

void GraphicsPath::AddBezier(core::Point p1, core::Point c1, core::Point c2, core::Point p2) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    if (!m_isFigureOpen || m_points.empty() || m_points.back() != p1) {
        m_points.push_back(p1);
        m_types.push_back(static_cast<uint8_t>(PathPointType::Start));
    }
    m_points.push_back(c1);
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(c2);
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_points.push_back(p2);
    m_types.push_back(static_cast<uint8_t>(PathPointType::Bezier));
    m_isFigureOpen = true;
}

void GraphicsPath::AddCurve(compat::span<const core::Point> points) {
    if (points.size() < 2) return;
    if (points.size() == 2) {
        AddLine(points[0], points[1]);
        return;
    }

    // Catmull-Rom 樣條曲線轉貝茲曲線逼近
    for (size_t i = 0; i + 1 < points.size(); ++i) {
        core::Point p0 = (i == 0) ? points[0] : points[i - 1];
        core::Point p1 = points[i];
        core::Point p2 = points[i + 1];
        core::Point p3 = (i + 2 < points.size()) ? points[i + 2] : p2;

        core::Point c1(p1.x + (p2.x - p0.x) / 6.0f, p1.y + (p2.y - p0.y) / 6.0f);
        core::Point c2(p2.x - (p3.x - p1.x) / 6.0f, p2.y - (p3.y - p1.y) / 6.0f);
        AddBezier(p1, c1, c2, p2);
    }
}

void GraphicsPath::AddPolygon(compat::span<const core::Point> points) {
    if (points.size() < 3) return;
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    m_points.push_back(points[0]);
    m_types.push_back(static_cast<uint8_t>(PathPointType::Start));

    for (size_t i = 1; i + 1 < points.size(); ++i) {
        m_points.push_back(points[i]);
        m_types.push_back(static_cast<uint8_t>(PathPointType::Line));
    }

    m_points.push_back(points.back());
    m_types.push_back(static_cast<uint8_t>(PathPointType::Line) | static_cast<uint8_t>(PathPointType::CloseSubpath));
    m_isFigureOpen = false;
}

void GraphicsPath::AddPath(const GraphicsPath& path, bool connect) {
    std::unique_lock<std::shared_mutex> lockThis(m_mutex, std::defer_lock);
    std::shared_lock<std::shared_mutex> lockOther(path.m_mutex, std::defer_lock);
    std::lock(lockThis, lockOther);

    if (path.m_points.empty()) return;

    for (size_t i = 0; i < path.m_points.size(); ++i) {
        uint8_t type = path.m_types[i];
        if (i == 0 && connect && m_isFigureOpen && !m_points.empty()) {
            type = static_cast<uint8_t>(PathPointType::Line);
        }
        m_points.push_back(path.m_points[i]);
        m_types.push_back(type);
    }
    m_isFigureOpen = path.m_isFigureOpen;
}

void GraphicsPath::CloseFigure() {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    if (!m_types.empty()) {
        m_types.back() |= static_cast<uint8_t>(PathPointType::CloseSubpath);
        m_isFigureOpen = false;
    }
}

void GraphicsPath::CloseAllFigures() {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    for (size_t i = 0; i < m_types.size(); ++i) {
        if (i + 1 == m_types.size() || (m_types[i + 1] & ~static_cast<uint8_t>(PathPointType::CloseSubpath)) == static_cast<uint8_t>(PathPointType::Start)) {
            m_types[i] |= static_cast<uint8_t>(PathPointType::CloseSubpath);
        }
    }
    m_isFigureOpen = false;
}

void GraphicsPath::StartFigure() {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_isFigureOpen = false;
}

void GraphicsPath::Reset() {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_points.clear();
    m_types.clear();
    m_isFigureOpen = false;
}

FillMode GraphicsPath::GetFillMode() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_fillMode;
}

void GraphicsPath::SetFillMode(FillMode mode) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_fillMode = mode;
}

core::Rect GraphicsPath::GetBounds() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (m_points.empty()) {
        return core::Rect(0.0f, 0.0f, 0.0f, 0.0f);
    }

    float minX = m_points[0].x;
    float maxX = m_points[0].x;
    float minY = m_points[0].y;
    float maxY = m_points[0].y;

    for (const auto& pt : m_points) {
        if (pt.x < minX) minX = pt.x;
        if (pt.x > maxX) maxX = pt.x;
        if (pt.y < minY) minY = pt.y;
        if (pt.y > maxY) maxY = pt.y;
    }
    return core::Rect(minX, minY, maxX - minX, maxY - minY);
}

size_t GraphicsPath::GetPointCount() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_points.size();
}

std::vector<core::Point> GraphicsPath::GetPathPoints() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_points;
}

std::vector<uint8_t> GraphicsPath::GetPathTypes() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_types;
}

std::vector<core::Point> GraphicsPath::Flatten(float flatness) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    std::vector<core::Point> flatPoints;
    if (m_points.empty()) return flatPoints;

    float flatnessSq = flatness * flatness;
    size_t i = 0;
    while (i < m_points.size()) {
        uint8_t type = m_types[i] & ~static_cast<uint8_t>(PathPointType::CloseSubpath);
        if (type == static_cast<uint8_t>(PathPointType::Start) || type == static_cast<uint8_t>(PathPointType::Line)) {
            flatPoints.push_back(m_points[i]);
            ++i;
        } else if (type == static_cast<uint8_t>(PathPointType::Bezier)) {
            if (i >= 1 && i + 2 < m_points.size()) {
                core::Point p1 = m_points[i - 1];
                core::Point c1 = m_points[i];
                core::Point c2 = m_points[i + 1];
                core::Point p2 = m_points[i + 2];
                SubdivideCubicBezier(p1, c1, c2, p2, flatnessSq, flatPoints);
                i += 3;
            } else {
                flatPoints.push_back(m_points[i]);
                ++i;
            }
        } else {
            flatPoints.push_back(m_points[i]);
            ++i;
        }
    }
    return flatPoints;
}

bool GraphicsPath::IsVisible(core::Point pt) const {
    std::vector<core::Point> flat = Flatten(0.5f);
    if (flat.size() < 3) return false;

    // 射線交叉點計算 (Ray casting)
    int32_t crossings = 0;
    for (size_t i = 0, j = flat.size() - 1; i < flat.size(); j = i++) {
        const auto& p1 = flat[i];
        const auto& p2 = flat[j];

        if (((p1.y > pt.y) != (p2.y > pt.y)) &&
            (pt.x < (p2.x - p1.x) * (pt.y - p1.y) / (p2.y - p1.y + 0.000001f) + p1.x)) {
            crossings++;
        }
    }
    return (crossings % 2) != 0;
}

bool GraphicsPath::IsVisible(float x, float y) const {
    return IsVisible(core::Point(x, y));
}

bool GraphicsPath::IsOutlineVisible(core::Point pt, const Pen& pen) const {
    std::vector<core::Point> flat = Flatten(0.5f);
    if (flat.size() < 2) return false;

    float halfW = pen.GetWidth() * 0.5f;
    float maxDistSq = (halfW < 1.0f ? 1.0f : halfW) * (halfW < 1.0f ? 1.0f : halfW);

    for (size_t i = 0; i + 1 < flat.size(); ++i) {
        if (DistanceSqToSegment(pt, flat[i], flat[i + 1]) <= maxDistSq) {
            return true;
        }
    }
    return false;
}

void GraphicsPath::Transform(float m11, float m12, float m21, float m22, float dx, float dy) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    for (auto& pt : m_points) {
        float nx = pt.x * m11 + pt.y * m21 + dx;
        float ny = pt.x * m12 + pt.y * m22 + dy;
        pt.x = nx;
        pt.y = ny;
    }
}

GraphicsPath GraphicsPath::Clone() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    GraphicsPath copy;
    copy.m_points       = m_points;
    copy.m_types        = m_types;
    copy.m_fillMode     = m_fillMode;
    copy.m_isFigureOpen = m_isFigureOpen;
    return copy;
}

} // namespace graphics
} // namespace cppui
