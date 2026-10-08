/*
 * File: CounterViewModel.hpp
 * Purpose: Declares CounterViewModel mediating between CounterModel and CounterForm.
 */

#pragma once

#include "ui/mvvm/ViewModelBase.hpp"
#include "ui/mvvm/RelayCommand.hpp"
#include "../models/CounterModel.hpp"
#include <memory>
#include <string>

/// <summary>
/// 計數器展示視圖模型 (ViewModel)
/// </summary>
class CounterViewModel : public cppui::mvvm::ViewModelBase {
public:
    /// <summary>
    /// 建構 CounterViewModel 並初始化指令與狀態。
    /// </summary>
    CounterViewModel();
    ~CounterViewModel() override = default;

    COMPAT_NODISCARD compat::string_view GetCountText() const noexcept;
    COMPAT_NODISCARD compat::string_view GetStatusMessage() const noexcept;

    std::shared_ptr<cppui::mvvm::RelayCommand> IncrementCommand;
    std::shared_ptr<cppui::mvvm::RelayCommand> DecrementCommand;
    std::shared_ptr<cppui::mvvm::RelayCommand> ResetCommand;

private:
    void UpdatePropertiesAndCommands();

    CounterModel m_model;
    std::string m_countText;
    std::string m_statusMessage;
};
