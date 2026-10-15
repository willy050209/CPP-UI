/**
 * @file LayoutManager.hpp
 * @brief 頂層排版調度管理器，協調 Measure 與 Arrange 排版週期及統計
 */

#pragma once

#include <cstdint>
#include "compat/Compat.hpp"
#include "ui/core/UIElement.hpp"

namespace cppui {
namespace layout {

/// <summary>
/// 頂層排版調度管理器 (LayoutManager)
/// 負責協調 Measure 與 Arrange 雙階段排版週期，並記錄排版調度統計數據。
/// </summary>
class LayoutManager {
public:
    /// <summary>
    /// 排版次數統計資訊結構。
    /// </summary>
    struct LayoutStats {
        uint64_t totalMeasures = 0;
        uint64_t totalArranges = 0;
    };

    /// <summary>
    /// 取得排版管理器全域單例。
    /// </summary>
    /// <returns>LayoutManager 引用。</returns>
    static LayoutManager& Get();

    /// <summary>
    /// 預設建構子。
    /// </summary>
    LayoutManager() = default;

    /// <summary>
    /// 預設解構子。
    /// </summary>
    ~LayoutManager() = default;

    /// <summary>
    /// 執行頂層根節點排版更新。
    /// </summary>
    /// <param name="root">排版樹根節點引用。</param>
    /// <param name="rootSize">可用根尺寸邊界。</param>
    void UpdateLayout(core::UIElement& root, core::Size rootSize);

    /// <summary>
    /// 取得當前排版統計資訊。
    /// </summary>
    /// <returns>LayoutStats 結構。</returns>
    COMPAT_NODISCARD LayoutStats GetStats() const noexcept { return m_stats; }

    /// <summary>
    /// 重置排版統計數據為零。
    /// </summary>
    void ResetStats() noexcept { m_stats = LayoutStats{}; }

private:
    LayoutStats m_stats;
};

} // namespace layout
} // namespace cppui
