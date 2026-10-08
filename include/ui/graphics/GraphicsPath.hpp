/**
 * @file GraphicsPath.hpp
 * @brief GDI+ 風格幾何路徑類別，支援線段、貝茲曲線、多邊形與執行緒安全自動同步
 */

#pragma once

#include <vector>
#include <shared_mutex>
#include <mutex>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/graphics/GraphicsTypes.hpp"

namespace cppui {
namespace graphics {

class Pen;

/// <summary>
/// 路徑控制點標記型別
/// </summary>
enum class PathPointType : uint8_t {
    Start = 0,
    Line = 1,
    Bezier = 3,
    CloseSubpath = 0x80
};

/// <summary>
/// 任意複合幾何輪廓路徑類別 (GraphicsPath)
/// 封裝直線、矩形、橢圓、圓弧與三次貝茲曲線，內建讀寫鎖支援多執行緒並發操作。
/// </summary>
class GraphicsPath {
public:
    /// <summary>預設建構子 (Alternate 填色模式)。</summary>
    GraphicsPath();

    /// <summary>
    /// 以指定填色模式建構幾何路徑。
    /// </summary>
    /// <param name="fillMode">填色規則 (Alternate 或 Winding)。</param>
    explicit GraphicsPath(FillMode fillMode);

    /// <summary>
    /// 從點序列陣列建構多邊形路徑。
    /// </summary>
    /// <param name="points">多邊形頂點跨度。</param>
    /// <param name="fillMode">填色規則。</param>
    explicit GraphicsPath(compat::span<const core::Point> points, FillMode fillMode = FillMode::Alternate);

    /// <summary>執行緒安全拷貝建構子。</summary>
    /// <param name="other">來源路徑。</param>
    GraphicsPath(const GraphicsPath& other);

    /// <summary>移動建構子。</summary>
    /// <param name="other">來源右值路徑。</param>
    GraphicsPath(GraphicsPath&& other) noexcept;

    /// <summary>執行緒安全拷貝賦值運算子。</summary>
    /// <param name="other">來源路徑。</param>
    /// <returns>自我引用。</returns>
    GraphicsPath& operator=(const GraphicsPath& other);

    /// <summary>移動賦值運算子。</summary>
    /// <param name="other">來源右值路徑。</param>
    /// <returns>自我引用。</returns>
    GraphicsPath& operator=(GraphicsPath&& other) noexcept;

    /// <summary>解構子。</summary>
    ~GraphicsPath() = default;

    /// <summary>
    /// 新增線段至路徑。
    /// </summary>
    /// <param name="p1">線段起點。</param>
    /// <param name="p2">線段終點。</param>
    void AddLine(core::Point p1, core::Point p2);

    /// <summary>
    /// 新增線段至路徑。
    /// </summary>
    /// <param name="x1">起點 X 座標。</param>
    /// <param name="y1">起點 Y 座標。</param>
    /// <param name="x2">終點 X 座標。</param>
    /// <param name="y2">終點 Y 座標。</param>
    void AddLine(float x1, float y1, float x2, float y2);

    /// <summary>
    /// 新增相連線段序列。
    /// </summary>
    /// <param name="points">折線頂點跨度。</param>
    void AddLines(compat::span<const core::Point> points);

    /// <summary>
    /// 新增矩形至路徑。
    /// </summary>
    /// <param name="rect">矩形邊界。</param>
    void AddRectangle(const core::Rect& rect);

    /// <summary>
    /// 新增圓角矩形至路徑。
    /// </summary>
    /// <param name="rect">矩形邊界。</param>
    /// <param name="radius">圓角半徑。</param>
    void AddRoundedRectangle(const core::Rect& rect, float radius);

    /// <summary>
    /// 新增橢圓至路徑。
    /// </summary>
    /// <param name="rect">外接矩形邊界。</param>
    void AddEllipse(const core::Rect& rect);

    /// <summary>
    /// 新增橢圓至路徑。
    /// </summary>
    /// <param name="x">左上角 X 座標。</param>
    /// <param name="y">左上角 Y 座標。</param>
    /// <param name="width">寬度。</param>
    /// <param name="height">高度。</param>
    void AddEllipse(float x, float y, float width, float height);

    /// <summary>
    /// 新增圓弧至路徑。
    /// </summary>
    /// <param name="rect">外接矩形邊界。</param>
    /// <param name="startAngle">起始角度 (度)。</param>
    /// <param name="sweepAngle">掃掠角度 (度)。</param>
    void AddArc(const core::Rect& rect, float startAngle, float sweepAngle);

    /// <summary>
    /// 新增三次貝茲曲線。
    /// </summary>
    /// <param name="p1">起點。</param>
    /// <param name="c1">第一控制點。</param>
    /// <param name="c2">第二控制點。</param>
    /// <param name="p2">終點。</param>
    void AddBezier(core::Point p1, core::Point c1, core::Point c2, core::Point p2);

    /// <summary>
    /// 新增曲線 (連續 Catmull-Rom 或三次曲線平滑逼近)。
    /// </summary>
    /// <param name="points">曲線控制點集合。</param>
    void AddCurve(compat::span<const core::Point> points);

    /// <summary>
    /// 新增閉合多邊形至路徑。
    /// </summary>
    /// <param name="points">多邊形頂點集合。</param>
    void AddPolygon(compat::span<const core::Point> points);

    /// <summary>
    /// 串接附加另一路徑之全部幾何圖元。
    /// </summary>
    /// <param name="path">待附加之來源路徑。</param>
    /// <param name="connect">是否以直線連接前一路徑終點與新路徑起點。</param>
    void AddPath(const GraphicsPath& path, bool connect = false);

    /// <summary>閉合當前幾何圖形。</summary>
    void CloseFigure();

    /// <summary>閉合路徑中所有未閉合之圖形。</summary>
    void CloseAllFigures();

    /// <summary>開啟新幾何圖形，下一點將作為新子路徑起點。</summary>
    void StartFigure();

    /// <summary>重設並清空所有幾何圖元與控制點。</summary>
    void Reset();

    /// <summary>取得填色模式。</summary>
    /// <returns>FillMode 列舉值。</returns>
    COMPAT_NODISCARD FillMode GetFillMode() const;

    /// <summary>設定填色模式。</summary>
    /// <param name="mode">新填色模式。</param>
    void SetFillMode(FillMode mode);

    /// <summary>取得路徑所有點之外接包圍矩形。</summary>
    /// <returns>外接矩形 Rect。</returns>
    COMPAT_NODISCARD core::Rect GetBounds() const;

    /// <summary>取得路徑中控制點總數。</summary>
    /// <returns>控制點數量。</returns>
    COMPAT_NODISCARD size_t GetPointCount() const;

    /// <summary>取得所有控制點副本。</summary>
    /// <returns>Point 向量。</returns>
    COMPAT_NODISCARD std::vector<core::Point> GetPathPoints() const;

    /// <summary>取得所有控制點型別標記。</summary>
    /// <returns>型別位元組向量。</returns>
    COMPAT_NODISCARD std::vector<uint8_t> GetPathTypes() const;

    /// <summary>
    /// 射線法命中測試：測試點是否落在封閉路徑內部。
    /// </summary>
    /// <param name="pt">待測試座標點。</param>
    /// <returns>若位於內部傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool IsVisible(core::Point pt) const;

    /// <summary>
    /// 射線法命中測試：測試點是否落在封閉路徑內部。
    /// </summary>
    /// <param name="x">X 座標。</param>
    /// <param name="y">Y 座標。</param>
    /// <returns>若位於內部傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool IsVisible(float x, float y) const;

    /// <summary>
    /// 描邊邊界命中測試：測試點是否落在畫筆描邊寬度範圍內。
    /// </summary>
    /// <param name="pt">待測試座標點。</param>
    /// <param name="pen">描邊畫筆引用。</param>
    /// <returns>若落在描邊線上傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool IsOutlineVisible(core::Point pt, const Pen& pen) const;

    /// <summary>
    /// 平整化曲線：將所有三次貝茲曲線與圓弧以折線逼近展開。
    /// </summary>
    /// <param name="flatness">容許最大平整度誤差。</param>
    /// <returns>展開後的連續折線點集合。</returns>
    COMPAT_NODISCARD std::vector<core::Point> Flatten(float flatness = 0.25f) const;

    /// <summary>
    /// 應用 2D 仿射矩陣變換至所有控制點。
    /// </summary>
    /// <param name="m11">X 縮放。</param>
    /// <param name="m12">Y 剪切。</param>
    /// <param name="m21">X 剪切。</param>
    /// <param name="m22">Y 縮放。</param>
    /// <param name="dx">X 平移。</param>
    /// <param name="dy">Y 平移。</param>
    void Transform(float m11, float m12, float m21, float m22, float dx, float dy);

    /// <summary>深拷貝幾何路徑。</summary>
    /// <returns>新的 GraphicsPath 物件。</returns>
    COMPAT_NODISCARD GraphicsPath Clone() const;

private:
    std::vector<core::Point>   m_points;
    std::vector<uint8_t>       m_types;
    FillMode                   m_fillMode{ FillMode::Alternate };
    bool                       m_isFigureOpen{ false };
    mutable std::shared_mutex  m_mutex;
};

} // namespace graphics
} // namespace cppui
