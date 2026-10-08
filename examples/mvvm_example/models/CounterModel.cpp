#include "CounterModel.hpp"

CounterModel::CounterModel(int initialValue, int minValue, int maxValue)
    : m_value(initialValue), m_minValue(minValue), m_maxValue(maxValue) {}

int CounterModel::GetValue() const noexcept {
    return m_value;
}

int CounterModel::GetMinValue() const noexcept {
    return m_minValue;
}

int CounterModel::GetMaxValue() const noexcept {
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
