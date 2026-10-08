#pragma once

#include "compat/compat.hpp"

class CounterModel {
public:
    explicit CounterModel(int initialValue = 0, int minValue = 0, int maxValue = 10);

    COMPAT_NODISCARD int GetValue() const noexcept;
    COMPAT_NODISCARD int GetMinValue() const noexcept;
    COMPAT_NODISCARD int GetMaxValue() const noexcept;

    COMPAT_NODISCARD bool CanIncrement() const noexcept;
    COMPAT_NODISCARD bool CanDecrement() const noexcept;

    bool Increment() noexcept;
    bool Decrement() noexcept;
    void Reset() noexcept;

private:
    int m_value;
    int m_minValue;
    int m_maxValue;
};
