/**
 * @file Grid.hpp
 * @brief 彈性網格容器，支援行列定義、像素/自動/星號比例求解與 Attached Properties
 */

#pragma once

#include <vector>
#include <unordered_map>
#include "compat/Compat.hpp"
#include "ui/layout/Panel.hpp"

namespace cppui {
namespace layout {

/// <summary>
/// 網格單元格度量單位
/// </summary>
enum class GridUnitType : uint8_t {
    Pixel, // 絕對像素值 (如 100px)
    Auto,  // 依內容度量自適應
    Star   // 按權重分配剩餘空間 (如 1*, 2*)
};

/// <summary>
/// 網格尺寸長度定義 (GridLength)
/// </summary>
struct GridLength {
    float        value = 1.0f;
    GridUnitType type = GridUnitType::Star;

    /// <summary>
    /// 預設建構子，建立 1* 之長度。
    /// </summary>
    GridLength() = default;

    /// <summary>
    /// 建構指定數值與單位型別之長度。
    /// </summary>
    /// <param name="val">長度數值或星號權重。</param>
    /// <param name="t">長度單位型別。</param>
    GridLength(float val, GridUnitType t) noexcept : value(val), type(t) {}

    /// <summary>
    /// 建立絕對像素長度。
    /// </summary>
    /// <param name="px">像素值。</param>
    /// <returns>GridLength 實例。</returns>
    COMPAT_NODISCARD static GridLength Pixel(float px) noexcept {
        return GridLength(px, GridUnitType::Pixel);
    }

    /// <summary>
    /// 建立內容自動自適應長度。
    /// </summary>
    /// <returns>GridLength 實例。</returns>
    COMPAT_NODISCARD static GridLength Auto() noexcept {
        return GridLength(1.0f, GridUnitType::Auto);
    }

    /// <summary>
    /// 建立星號比例長度。
    /// </summary>
    /// <param name="weight">權重比例值，預設為 1.0f。</param>
    /// <returns>GridLength 實例。</returns>
    COMPAT_NODISCARD static GridLength Star(float weight = 1.0f) noexcept {
        return GridLength(weight, GridUnitType::Star);
    }
};

/// <summary>
/// 網格列定義 (RowDefinition)
/// </summary>
struct RowDefinition {
    GridLength height = GridLength::Star(1.0f);
    float      minHeight = 0.0f;
    float      maxHeight = 1000000.0f;
    float      actualHeight = 0.0f;

    /// <summary>
    /// 預設建構子，建立 1* 高度之列。
    /// </summary>
    RowDefinition() = default;

    /// <summary>
    /// 建構指定高度之列。
    /// </summary>
    /// <param name="h">列高度定義。</param>
    explicit RowDefinition(GridLength h) noexcept : height(h) {}

    /// <summary>
    /// 建構具備最小/最大高度約束之列。
    /// </summary>
    /// <param name="h">列高度定義。</param>
    /// <param name="minH">最小高度像素。</param>
    /// <param name="maxH">最大高度像素。</param>
    RowDefinition(GridLength h, float minH, float maxH) noexcept
        : height(h), minHeight(minH), maxHeight(maxH) {}
};

/// <summary>
/// 網格欄定義 (ColumnDefinition)
/// </summary>
struct ColumnDefinition {
    GridLength width = GridLength::Star(1.0f);
    float      minWidth = 0.0f;
    float      maxWidth = 1000000.0f;
    float      actualWidth = 0.0f;

    /// <summary>
    /// 預設建構子，建立 1* 寬度之欄。
    /// </summary>
    ColumnDefinition() = default;

    /// <summary>
    /// 建構指定寬度之欄。
    /// </summary>
    /// <param name="w">欄寬度定義。</param>
    explicit ColumnDefinition(GridLength w) noexcept : width(w) {}

    /// <summary>
    /// 建構具備最小/最大寬度約束之欄。
    /// </summary>
    /// <param name="w">欄寬度定義。</param>
    /// <param name="minW">最小寬度像素。</param>
    /// <param name="maxW">最大寬度像素。</param>
    ColumnDefinition(GridLength w, float minW, float maxW) noexcept
        : width(w), minWidth(minW), maxWidth(maxW) {}
};

/// <summary>
/// 彈性網格容器 (Grid)
/// 支援多行列排版、Pixel / Auto / Star 比例求解與 SetRow / SetColumn 附加屬性。
/// </summary>
class Grid : public Panel {
public:
    /// <summary>
    /// 建構網格排版容器。
    /// </summary>
    Grid() = default;

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    ~Grid() override = default;

    // --- 行列定義註冊 ---

    /// <summary>
    /// 加入列定義 (RowDefinition)。
    /// </summary>
    /// <param name="row">列定義物件。</param>
    void AddRow(RowDefinition row);

    /// <summary>
    /// 便捷多載：以指定高度加入列。
    /// </summary>
    /// <param name="height">高度長度定義。</param>
    void AddRow(GridLength height) { AddRow(RowDefinition(height)); }

    /// <summary>
    /// 加入列定義別名。
    /// </summary>
    /// <param name="row">列定義物件。</param>
    void AddRowDefinition(RowDefinition row) { AddRow(row); }

    /// <summary>
    /// 加入欄定義 (ColumnDefinition)。
    /// </summary>
    /// <param name="col">欄定義物件。</param>
    void AddColumn(ColumnDefinition col);

    /// <summary>
    /// 便捷多載：以指定寬度加入欄。
    /// </summary>
    /// <param name="width">寬度長度定義。</param>
    void AddColumn(GridLength width) { AddColumn(ColumnDefinition(width)); }

    /// <summary>
    /// 加入欄定義別名。
    /// </summary>
    /// <param name="col">欄定義物件。</param>
    void AddColumnDefinition(ColumnDefinition col) { AddColumn(col); }

    /// <summary>
    /// 取得所有列定義清單。
    /// </summary>
    /// <returns>列定義清單常數引用。</returns>
    COMPAT_NODISCARD const std::vector<RowDefinition>& GetRowDefinitions() const noexcept {
        return m_rowDefinitions;
    }

    /// <summary>
    /// 取得所有欄定義清單。
    /// </summary>
    /// <returns>欄定義清單常數引用。</returns>
    COMPAT_NODISCARD const std::vector<ColumnDefinition>& GetColumnDefinitions() const noexcept {
        return m_columnDefinitions;
    }

    // --- 附加屬性 (Attached Properties) ---

    /// <summary>
    /// 設定元件所在的網格列索引 (附加屬性)。
    /// </summary>
    /// <param name="element">目標 UI 元件。</param>
    /// <param name="row">列索引 (0-based)。</param>
    static void SetRow(core::UIElement& element, size_t row);

    /// <summary>
    /// 取得元件所在的網格列索引 (附加屬性)。
    /// </summary>
    /// <param name="element">目標 UI 元件。</param>
    /// <returns>列索引 (0-based)。</returns>
    COMPAT_NODISCARD static size_t GetRow(const core::UIElement& element) noexcept;

    /// <summary>
    /// 設定元件所在的網格欄索引 (附加屬性)。
    /// </summary>
    /// <param name="element">目標 UI 元件。</param>
    /// <param name="col">欄索引 (0-based)。</param>
    static void SetColumn(core::UIElement& element, size_t col);

    /// <summary>
    /// 取得元件所在的網格欄索引 (附加屬性)。
    /// </summary>
    /// <param name="element">目標 UI 元件。</param>
    /// <returns>欄索引 (0-based)。</returns>
    COMPAT_NODISCARD static size_t GetColumn(const core::UIElement& element) noexcept;

    /// <summary>
    /// 設定元件跨越的列數 (附加屬性)。
    /// </summary>
    /// <param name="element">目標 UI 元件。</param>
    /// <param name="span">跨越列數。</param>
    static void SetRowSpan(core::UIElement& element, size_t span);

    /// <summary>
    /// 取得元件跨越的列數 (附加屬性)。
    /// </summary>
    /// <param name="element">目標 UI 元件。</param>
    /// <returns>跨越列數。</returns>
    COMPAT_NODISCARD static size_t GetRowSpan(const core::UIElement& element) noexcept;

    /// <summary>
    /// 設定元件跨越的欄數 (附加屬性)。
    /// </summary>
    /// <param name="element">目標 UI 元件。</param>
    /// <param name="span">跨越欄數。</param>
    static void SetColumnSpan(core::UIElement& element, size_t span);

    /// <summary>
    /// 取得元件跨越的欄數 (附加屬性)。
    /// </summary>
    /// <param name="element">目標 UI 元件。</param>
    /// <returns>跨越欄數。</returns>
    COMPAT_NODISCARD static size_t GetColumnSpan(const core::UIElement& element) noexcept;

    /// <summary>
    /// 覆寫排版測量階段，求解 Pixel、Auto 與 Star 比例尺寸。
    /// </summary>
    /// <param name="availableSize">可用尺寸。</param>
    /// <returns>網格期望尺寸。</returns>
    core::Size MeasureOverride(core::Size availableSize) override;

    /// <summary>
    /// 覆寫排版排列階段，依據行列佈局配置各子元件之矩形空間。
    /// </summary>
    /// <param name="finalSize">實際分配尺寸。</param>
    void ArrangeOverride(core::Size finalSize) override;

private:
    struct CellInfo {
        size_t row = 0;
        size_t col = 0;
        size_t rowSpan = 1;
        size_t colSpan = 1;
    };

    static std::unordered_map<const core::UIElement*, CellInfo>& GetAttachedStorage();

    std::vector<RowDefinition>    m_rowDefinitions;
    std::vector<ColumnDefinition> m_columnDefinitions;
};

} // namespace layout
} // namespace cppui
