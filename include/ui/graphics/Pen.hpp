/**
 * @file Pen.hpp
 * @brief GDI+ 風格幾何畫筆類別，具備執行緒安全自動同步
 */

#pragma once

#include <shared_mutex>
#include <mutex>
#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/graphics/GraphicsTypes.hpp"

namespace cppui {
namespace graphics {

class Brush;

/// <summary>
/// 幾何圖形輪廓描邊畫筆類別 (Pen)
/// 封裝線條色彩、粗細、端點與連接處樣式，內建讀寫鎖支援多執行緒並發存取。
/// </summary>
class Pen {
public:
    /// <summary>
    /// 以指定色彩與線寬建構畫筆。
    /// </summary>
    /// <param name="color">線條色彩。</param>
    /// <param name="width">線條寬度，必須大於或等於 0.0f。</param>
    /// <exception cref="std::invalid_argument">若 width 小於 0.0f 擲出例外。</exception>
    explicit Pen(core::Color color, float width = 1.0f);

    /// <summary>
    /// 以指定畫刷與線寬建構畫筆。
    /// </summary>
    /// <param name="brush">用以取得色彩的畫刷引用。</param>
    /// <param name="width">線條寬度，必須大於或等於 0.0f。</param>
    /// <exception cref="std::invalid_argument">若 width 小於 0.0f 擲出例外。</exception>
    explicit Pen(const Brush& brush, float width = 1.0f);

    /// <summary>
    /// 執行緒安全拷貝建構子。
    /// </summary>
    /// <param name="other">來源畫筆。</param>
    Pen(const Pen& other);

    /// <summary>
    /// 移動建構子。
    /// </summary>
    /// <param name="other">來源右值畫筆。</param>
    Pen(Pen&& other) noexcept;

    /// <summary>
    /// 執行緒安全拷貝賦值運算子，採用雙物件鎖定避免死鎖。
    /// </summary>
    /// <param name="other">來源畫筆。</param>
    /// <returns>自我引用。</returns>
    Pen& operator=(const Pen& other);

    /// <summary>
    /// 移動賦值運算子。
    /// </summary>
    /// <param name="other">來源右值畫筆。</param>
    /// <returns>自我引用。</returns>
    Pen& operator=(Pen&& other) noexcept;

    /// <summary>解構子。</summary>
    ~Pen() = default;

    /// <summary>
    /// 取得畫筆色彩。
    /// </summary>
    /// <returns>目前線條色彩。</returns>
    COMPAT_NODISCARD core::Color GetColor() const;

    /// <summary>
    /// 設定畫筆色彩。
    /// </summary>
    /// <param name="color">新線條色彩。</param>
    void SetColor(core::Color color);

    /// <summary>
    /// 取得畫筆寬度。
    /// </summary>
    /// <returns>線條寬度像素值。</returns>
    COMPAT_NODISCARD float GetWidth() const;

    /// <summary>
    /// 設定畫筆寬度。
    /// </summary>
    /// <param name="width">新線條寬度，必須大於或等於 0.0f。</param>
    /// <exception cref="std::invalid_argument">若 width 小於 0.0f 擲出例外。</exception>
    void SetWidth(float width);

    /// <summary>
    /// 取得虛線樣式。
    /// </summary>
    /// <returns>目前虛線樣式。</returns>
    COMPAT_NODISCARD DashStyle GetDashStyle() const;

    /// <summary>
    /// 設定虛線樣式。
    /// </summary>
    /// <param name="style">新虛線樣式。</param>
    void SetDashStyle(DashStyle style);

    /// <summary>
    /// 取得線段端點樣式。
    /// </summary>
    /// <returns>目前端點樣式。</returns>
    COMPAT_NODISCARD LineCap GetLineCap() const;

    /// <summary>
    /// 設定線段端點樣式。
    /// </summary>
    /// <param name="cap">新端點樣式。</param>
    void SetLineCap(LineCap cap);

    /// <summary>
    /// 取得線段轉角連接樣式。
    /// </summary>
    /// <returns>目前連接樣式。</returns>
    COMPAT_NODISCARD LineJoin GetLineJoin() const;

    /// <summary>
    /// 設定線段轉角連接樣式。
    /// </summary>
    /// <param name="join">新連接樣式。</param>
    void SetLineJoin(LineJoin join);

    /// <summary>
    /// 複製當前畫筆為獨立副本。
    /// </summary>
    /// <returns>新的 Pen 物件。</returns>
    COMPAT_NODISCARD Pen Clone() const;

private:
    core::Color                m_color{ 0, 0, 0, 255 };
    float                      m_width{ 1.0f };
    DashStyle                  m_dashStyle{ DashStyle::Solid };
    LineCap                    m_lineCap{ LineCap::Flat };
    LineJoin                   m_lineJoin{ LineJoin::Miter };
    mutable compat::shared_mutex m_mutex;
};

} // namespace graphics
} // namespace cppui
