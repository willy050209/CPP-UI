/*
 * File: CounterModel.cpp
 * Purpose: Implements CounterModel bounds checking and state manipulation methods.
 */

#include "CounterModel.hpp"
#include <cstdint>

CounterModel::CounterModel(int32_t initialValue, int32_t minValue, int32_t maxValue)
    : m_value(initialValue), m_minValue(minValue), m_maxValue(maxValue) {}

int32_t CounterModel::GetValue() const noexcept {
    return m_value;
}

int32_t CounterModel::GetMinValue() const noexcept {
    return m_minValue;
}

int32_t CounterModel::GetMaxValue() const noexcept {
    return m_maxValue;
}

bool CounterModel::CanIncrement() const noexcept {
    return m_value < m_maxValue;
}

bool CounterModel::CanDecrement() const noexcept {
    return m_value > m_minValue;
}

bool CounterModel::Increment() noexcept {
    if (CanIncrement()) {
        m_value++;
        return true;
    }
    return false;
}

bool CounterModel::Decrement() noexcept {
    if (CanDecrement()) {
        m_value--;
        return true;
    }
    return false;
}

void CounterModel::Reset() noexcept {
    m_value = m_minValue;
}
