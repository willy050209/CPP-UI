#include "CounterViewModel.hpp"

CounterViewModel::CounterViewModel()
    : m_model(0, 0, 10) {

    IncrementCommand = cppui::mvvm::MakeCommand(
        [this]() {
            if (m_model.Increment()) {
                UpdatePropertiesAndCommands();
            }
        },
        [this]() -> bool {
            return m_model.CanIncrement();
        }
    );

    DecrementCommand = cppui::mvvm::MakeCommand(
        [this]() {
            if (m_model.Decrement()) {
                UpdatePropertiesAndCommands();
            }
        },
        [this]() -> bool {
            return m_model.CanDecrement();
        }
    );

    ResetCommand = cppui::mvvm::MakeCommand(
        [this]() {
            m_model.Reset();
                UpdatePropertiesAndCommands();
        },
        [this]() -> bool {
            return m_model.GetValue() != m_model.GetMinValue();
        }
    );

    UpdatePropertiesAndCommands();
}

compat::string_view CounterViewModel::GetCountText() const noexcept {
    return m_countText;
}

compat::string_view CounterViewModel::GetStatusMessage() const noexcept {
    return m_statusMessage;
}

void CounterViewModel::UpdatePropertiesAndCommands() {
    std::string newCount = "計數數值: " + std::to_string(m_model.GetValue());
    SetProperty(m_countText, newCount, "CountText");

    std::string newStatus;
    if (!m_model.CanIncrement()) {
        newStatus = "狀態：已達最大上限 (10)，無法繼續增加！";
    } else if (!m_model.CanDecrement()) {
        newStatus = "狀態：已處於初始最小值 (0)，無法繼續減少。";
    } else {
        newStatus = "狀態：正常計數範圍內 (0 ~ 10)。";
    }
    SetProperty(m_statusMessage, newStatus, "StatusMessage");

    if (IncrementCommand) IncrementCommand->NotifyCanExecuteChanged();
    if (DecrementCommand) DecrementCommand->NotifyCanExecuteChanged();
    if (ResetCommand) ResetCommand->NotifyCanExecuteChanged();
}
