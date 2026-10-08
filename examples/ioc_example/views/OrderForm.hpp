/*
 * File: OrderForm.hpp
 * Purpose: Declares OrderForm view for the IoC sample application.
 */

#pragma once

#include "ui/app/Form.hpp"
#include "../viewmodels/OrderViewModel.hpp"
#include <memory>

namespace cppui {
namespace controls {
class TextBlock;
} // namespace controls
} // namespace cppui

/// <summary>
/// 訂單管理表單畫面
/// </summary>
class OrderForm : public cppui::app::Form {
public:
    /// <summary>
    /// 建構 OrderForm 並綁定 ViewModel。
    /// </summary>
    /// <param name="viewModel">訂單展示視圖模型。</param>
    explicit OrderForm(std::shared_ptr<OrderViewModel> viewModel);
    ~OrderForm() override = default;

private:
    std::shared_ptr<OrderViewModel> m_viewModel;
    std::shared_ptr<cppui::controls::TextBlock> m_summaryLabel;
    std::shared_ptr<cppui::controls::TextBlock> m_actionLabel;
};
