/**
 * @file CounterForm.hpp
 * @brief MVVM 範例之計數器視窗表單視圖宣告
 */

#pragma once

#include "ui/app/Form.hpp"
#include "../viewmodels/CounterViewModel.hpp"
#include <memory>

namespace cppui {
namespace controls {
class TextBlock;
} // namespace controls
} // namespace cppui

/// <summary>
/// 計數器範例表單視圖，展示 MVVM 屬性變更與命令繫結機制。
/// </summary>
class CounterForm : public cppui::app::Form {
public:
    /// <summary>
    /// 建構計數器表單視圖。
    /// </summary>
    /// <param name="viewModel">繫結之 CounterViewModel 實例。</param>
    explicit CounterForm(std::shared_ptr<CounterViewModel> viewModel);

    ~CounterForm() override = default;

private:
    std::shared_ptr<CounterViewModel> m_viewModel;
    std::shared_ptr<cppui::controls::TextBlock> m_countLabel;
    std::shared_ptr<cppui::controls::TextBlock> m_statusLabel;
};
