/*
 * File: CounterModel.hpp
 * Purpose: Declares CounterModel domain entity managing counter value bounds and mutation logic.
 */

#pragma once

#include <cstdint>
#include "compat/Compat.hpp"

/// <summary>
/// 計數器領域模型 (Domain Model)
/// 負責維護計數器數值與上下限邊界條件之業務規則。
/// </summary>
class CounterModel {
public:
    /// <summary>
    /// 建構 CounterModel 實例。
    /// </summary>
    /// <param name="initialValue">初始數值。</param>
    /// <param name="minValue">數值下限。</param>
    /// <param name="maxValue">數值上限。</param>
    explicit CounterModel(int32_t initialValue = 0, int32_t minValue = 0, int32_t maxValue = 10);

    COMPAT_NODISCARD int32_t GetValue() const noexcept;
    COMPAT_NODISCARD int32_t GetMinValue() const noexcept;
    COMPAT_NODISCARD int32_t GetMaxValue() const noexcept;

    COMPAT_NODISCARD bool CanIncrement() const noexcept;
    COMPAT_NODISCARD bool CanDecrement() const noexcept;

    bool Increment() noexcept;
    bool Decrement() noexcept;
    void Reset() noexcept;

private:
    int32_t m_value;
    int32_t m_minValue;
    int32_t m_maxValue;
};
