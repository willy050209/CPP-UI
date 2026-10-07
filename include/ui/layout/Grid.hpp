#pragma once

#include <vector>
#include <unordered_map>
#include "compat/compat.hpp"
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

    GridLength() = default;
    GridLength(float val, GridUnitType t) noexcept : value(val), type(t) {}

    COMPAT_NODISCARD static GridLength Pixel(float px) noexcept {
        return GridLength(px, GridUnitType::Pixel);
    }
    COMPAT_NODISCARD static GridLength Auto() noexcept {
        return GridLength(1.0f, GridUnitType::Auto);
    }
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

    RowDefinition() = default;
    explicit RowDefinition(GridLength h) noexcept : height(h) {}
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

    ColumnDefinition() = default;
    explicit ColumnDefinition(GridLength w) noexcept : width(w) {}
    ColumnDefinition(GridLength w, float minW, float maxW) noexcept
        : width(w), minWidth(minW), maxWidth(maxW) {}
};

/// <summary>
/// 彈性網格容器 (Grid)
/// 支援多行列排版、Pixel / Auto / Star 比例求解與 SetRow / SetColumn 附加屬性。
/// </summary>
class Grid : public Panel {
public:
    Grid() = default;
    ~Grid() override = default;

    // --- 行列定義註冊 ---
    void AddRow(RowDefinition row);
    void AddRow(GridLength height) { AddRow(RowDefinition(height)); }
    void AddRowDefinition(RowDefinition row) { AddRow(row); }

    void AddColumn(ColumnDefinition col);
    void AddColumn(GridLength width) { AddColumn(ColumnDefinition(width)); }
    void AddColumnDefinition(ColumnDefinition col) { AddColumn(col); }

    COMPAT_NODISCARD const std::vector<RowDefinition>& GetRowDefinitions() const noexcept {
        return m_rowDefinitions;
    }
    COMPAT_NODISCARD const std::vector<ColumnDefinition>& GetColumnDefinitions() const noexcept {
        return m_columnDefinitions;
    }

    // --- 附加屬性 (Attached Properties) ---
    static void SetRow(core::UIElement& element, size_t row);
    COMPAT_NODISCARD static size_t GetRow(const core::UIElement& element) noexcept;

    static void SetColumn(core::UIElement& element, size_t col);
    COMPAT_NODISCARD static size_t GetColumn(const core::UIElement& element) noexcept;

    static void SetRowSpan(core::UIElement& element, size_t span);
    COMPAT_NODISCARD static size_t GetRowSpan(const core::UIElement& element) noexcept;

    static void SetColumnSpan(core::UIElement& element, size_t span);
    COMPAT_NODISCARD static size_t GetColumnSpan(const core::UIElement& element) noexcept;

    core::Size MeasureOverride(core::Size availableSize) override;
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
