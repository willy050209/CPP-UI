#pragma once

#include <cstdint>
#include "compat/compat.hpp"
#include "ui/core/UIElement.hpp"

namespace cppui {
namespace layout {

/// <summary>
/// 頂層排版調度管理器 (LayoutManager)
/// 負責協調 Measure 與 Arrange 雙階段排版週期，並記錄排版調度統計數據。
/// </summary>
class LayoutManager {
public:
    struct LayoutStats {
        uint64_t totalMeasures = 0;
        uint64_t totalArranges = 0;
    };

    static LayoutManager& Get();

    LayoutManager() = default;
    ~LayoutManager() = default;

    /// <summary>
    /// 執行頂層根節點排版更新
    /// </summary>
    void UpdateLayout(core::UIElement& root, core::Size rootSize);

    COMPAT_NODISCARD LayoutStats GetStats() const noexcept { return m_stats; }
    void ResetStats() noexcept { m_stats = LayoutStats{}; }

private:
    LayoutStats m_stats;
};

} // namespace layout
} // namespace cppui
