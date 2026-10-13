/**
 * @file Brush.hpp
 * @brief GDI+ 風格畫刷抽象基底與衍生具體實作 (SolidBrush, LinearGradientBrush, TextureBrush)
 */

#pragma once

#include <memory>
#include <shared_mutex>
#include <mutex>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/graphics/GraphicsTypes.hpp"

namespace cppui {
namespace graphics {

class Bitmap;

/// <summary>
/// 幾何圖形填色畫刷抽象基底類別 (Brush)
/// </summary>
class Brush {
public:
    /// <summary>虛擬解構子。</summary>
    virtual ~Brush() = default;

    /// <summary>
    /// 取得畫刷具體型別。
    /// </summary>
    /// <returns>BrushType 列舉值。</returns>
    COMPAT_NODISCARD virtual BrushType GetType() const noexcept = 0;

    /// <summary>
    /// 複製當前畫刷為獨立多型副本。
    /// </summary>
    /// <returns>新建立之畫刷唯一擁有權指標。</returns>
    COMPAT_NODISCARD virtual std::unique_ptr<Brush> Clone() const = 0;
};

/// <summary>
/// 純色填色畫刷 (SolidBrush)
/// 內建讀寫鎖支援多執行緒並發存取。
/// </summary>
class SolidBrush : public Brush {
public:
    /// <summary>
    /// 以指定色彩建構純色畫刷。
    /// </summary>
    /// <param name="color">填色色彩。</param>
    explicit SolidBrush(core::Color color);

    /// <summary>執行緒安全拷貝建構子。</summary>
    /// <param name="other">來源畫刷。</param>
    SolidBrush(const SolidBrush& other);

    /// <summary>移動建構子。</summary>
    /// <param name="other">來源右值畫刷。</param>
    SolidBrush(SolidBrush&& other) noexcept;

    /// <summary>執行緒安全拷貝賦值運算子。</summary>
    /// <param name="other">來源畫刷。</param>
    /// <returns>自我引用。</returns>
    SolidBrush& operator=(const SolidBrush& other);

    /// <summary>移動賦值運算子。</summary>
    /// <param name="other">來源右值畫刷。</param>
    /// <returns>自我引用。</returns>
    SolidBrush& operator=(SolidBrush&& other) noexcept;

    /// <summary>解構子。</summary>
    ~SolidBrush() override = default;

    /// <summary>
    /// 取得填色色彩。
    /// </summary>
    /// <returns>目前填色色彩。</returns>
    COMPAT_NODISCARD core::Color GetColor() const;

    /// <summary>
    /// 設定填色色彩。
    /// </summary>
    /// <param name="color">新填色色彩。</param>
    void SetColor(core::Color color);

    /// <summary>取得畫刷型別。</summary>
    /// <returns>BrushType::SolidColor。</returns>
    COMPAT_NODISCARD BrushType GetType() const noexcept override {
        return BrushType::SolidColor;
    }

    /// <summary>深拷貝純色畫刷。</summary>
    /// <returns>新建立之 SolidBrush 物件。</returns>
    COMPAT_NODISCARD std::unique_ptr<Brush> Clone() const override;

private:
    core::Color                m_color;
    mutable compat::shared_mutex m_mutex;
};

/// <summary>
/// 線性漸層填色畫刷 (LinearGradientBrush)
/// 內建讀寫鎖支援多執行緒並發存取。
/// </summary>
class LinearGradientBrush : public Brush {
public:
    /// <summary>
    /// 依據起訖座標與雙色建立線性漸層畫刷。
    /// </summary>
    /// <param name="p1">漸層起始座標。</param>
    /// <param name="p2">漸層終止座標。</param>
    /// <param name="c1">起始色彩。</param>
    /// <param name="c2">終止色彩。</param>
    LinearGradientBrush(core::Point p1, core::Point p2, core::Color c1, core::Color c2);

    /// <summary>
    /// 依據矩形區域、雙色與旋轉角度建立線性漸層畫刷。
    /// </summary>
    /// <param name="rect">漸層覆蓋矩形。</param>
    /// <param name="c1">起始色彩。</param>
    /// <param name="c2">終止色彩。</param>
    /// <param name="angleDegrees">漸層角度 (0~360度)。</param>
    LinearGradientBrush(const core::Rect& rect, core::Color c1, core::Color c2, float angleDegrees);

    /// <summary>執行緒安全拷貝建構子。</summary>
    /// <param name="other">來源畫刷。</param>
    LinearGradientBrush(const LinearGradientBrush& other);

    /// <summary>移動建構子。</summary>
    /// <param name="other">來源右值畫刷。</param>
    LinearGradientBrush(LinearGradientBrush&& other) noexcept;

    /// <summary>執行緒安全拷貝賦值運算子。</summary>
    /// <param name="other">來源畫刷。</param>
    /// <returns>自我引用。</returns>
    LinearGradientBrush& operator=(const LinearGradientBrush& other);

    /// <summary>移動賦值運算子。</summary>
    /// <param name="other">來源右值畫刷。</param>
    /// <returns>自我引用。</returns>
    LinearGradientBrush& operator=(LinearGradientBrush&& other) noexcept;

    /// <summary>解構子。</summary>
    ~LinearGradientBrush() override = default;

    /// <summary>取得漸層起始座標。</summary>
    /// <returns>起始座標點。</returns>
    COMPAT_NODISCARD core::Point GetStartPoint() const;

    /// <summary>設定漸層起始座標。</summary>
    /// <param name="p">新起始座標點。</param>
    void SetStartPoint(core::Point p);

    /// <summary>取得漸層終止座標。</summary>
    /// <returns>終止座標點。</returns>
    COMPAT_NODISCARD core::Point GetEndPoint() const;

    /// <summary>設定漸層終止座標。</summary>
    /// <param name="p">新終止座標點。</param>
    void SetEndPoint(core::Point p);

    /// <summary>取得起始色彩。</summary>
    /// <returns>起始色彩。</returns>
    COMPAT_NODISCARD core::Color GetStartColor() const;

    /// <summary>設定起始色彩。</summary>
    /// <param name="c">新起始色彩。</param>
    void SetStartColor(core::Color c);

    /// <summary>取得終止色彩。</summary>
    /// <returns>終止色彩。</returns>
    COMPAT_NODISCARD core::Color GetEndColor() const;

    /// <summary>設定終止色彩。</summary>
    /// <param name="c">新終止色彩。</param>
    void SetEndColor(core::Color c);

    /// <summary>取得畫刷型別。</summary>
    /// <returns>BrushType::LinearGradient。</returns>
    COMPAT_NODISCARD BrushType GetType() const noexcept override {
        return BrushType::LinearGradient;
    }

    /// <summary>深拷貝漸層畫刷。</summary>
    /// <returns>新建立之 LinearGradientBrush 物件。</returns>
    COMPAT_NODISCARD std::unique_ptr<Brush> Clone() const override;

    /// <summary>
    /// 計算二維座標點於此線性漸層定義下的色彩插值結果。
    /// </summary>
    /// <param name="pt">二維空間座標點。</param>
    /// <returns>插值計算後之 Color。</returns>
    COMPAT_NODISCARD core::Color GetColorAt(core::Point pt) const;

private:
    core::Point                m_startPoint;
    core::Point                m_endPoint;
    core::Color                m_startColor;
    core::Color                m_endColor;
    mutable compat::shared_mutex m_mutex;
};

/// <summary>
/// 點陣材質填色畫刷 (TextureBrush)
/// 內建讀寫鎖支援多執行緒並發存取。
/// </summary>
class TextureBrush : public Brush {
public:
    /// <summary>
    /// 以指定點陣影像建構材質畫刷。
    /// </summary>
    /// <param name="image">材質來源影像共享指標。</param>
    /// <exception cref="std::invalid_argument">若 image 為 nullptr 擲出例外。</exception>
    explicit TextureBrush(std::shared_ptr<Bitmap> image);

    /// <summary>執行緒安全拷貝建構子。</summary>
    /// <param name="other">來源畫刷。</param>
    TextureBrush(const TextureBrush& other);

    /// <summary>移動建構子。</summary>
    /// <param name="other">來源右值畫刷。</param>
    TextureBrush(TextureBrush&& other) noexcept;

    /// <summary>執行緒安全拷貝賦值運算子。</summary>
    /// <param name="other">來源畫刷。</param>
    /// <returns>自我引用。</returns>
    TextureBrush& operator=(const TextureBrush& other);

    /// <summary>移動賦值運算子。</summary>
    /// <param name="other">來源右值畫刷。</param>
    /// <returns>自我引用。</returns>
    TextureBrush& operator=(TextureBrush&& other) noexcept;

    /// <summary>解構子。</summary>
    ~TextureBrush() override = default;

    /// <summary>取得材質影像指標。</summary>
    /// <returns>材質 Bitmap 共享指標。</returns>
    COMPAT_NODISCARD std::shared_ptr<Bitmap> GetImage() const;

    /// <summary>取得畫刷型別。</summary>
    /// <returns>BrushType::Texture。</returns>
    COMPAT_NODISCARD BrushType GetType() const noexcept override {
        return BrushType::Texture;
    }

    /// <summary>深拷貝材質畫刷。</summary>
    /// <returns>新建立之 TextureBrush 物件。</returns>
    COMPAT_NODISCARD std::unique_ptr<Brush> Clone() const override;

private:
    std::shared_ptr<Bitmap>    m_image;
    mutable compat::shared_mutex m_mutex;
};

} // namespace graphics
} // namespace cppui
