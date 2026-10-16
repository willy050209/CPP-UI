/**
 * @file SelectionModel.cpp
 * @brief 設計表面選取模型實作
 */

#include "ui/designer/surface/SelectionModel.hpp"
#include <unordered_set>
#include <algorithm>

namespace cppui {
namespace designer {
namespace surface {

struct SelectionModel::Impl {
    std::vector<std::string> selectedUuids;
    std::unordered_set<std::string> selectedSet;
    DesignerEvent<std::vector<std::string>> selectionChangedSignal;

    void NotifyChanged() {
        selectionChangedSignal.Notify(selectedUuids);
    }
};

SelectionModel::SelectionModel() : m_impl(std::make_unique<Impl>()) {}
SelectionModel::~SelectionModel() = default;
SelectionModel::SelectionModel(SelectionModel&& other) noexcept = default;
SelectionModel& SelectionModel::operator=(SelectionModel&& other) noexcept = default;

void SelectionModel::SetSelection(std::string nodeUuid) {
    if (m_impl->selectedUuids.size() == 1 && m_impl->selectedUuids[0] == nodeUuid) {
        return; // 無變更
    }

    m_impl->selectedUuids.clear();
    m_impl->selectedSet.clear();

    if (!nodeUuid.empty()) {
        m_impl->selectedSet.insert(nodeUuid);
        m_impl->selectedUuids.push_back(std::move(nodeUuid));
    }

    m_impl->NotifyChanged();
}

void SelectionModel::AddToSelection(std::string nodeUuid) {
    if (nodeUuid.empty() || m_impl->selectedSet.count(nodeUuid) > 0) {
        return;
    }

    m_impl->selectedSet.insert(nodeUuid);
    m_impl->selectedUuids.push_back(std::move(nodeUuid));
    m_impl->NotifyChanged();
}

void SelectionModel::RemoveFromSelection(compat::string_view nodeUuid) {
    std::string key(nodeUuid.data(), nodeUuid.size());
    auto it = m_impl->selectedSet.find(key);
    if (it == m_impl->selectedSet.end()) {
        return;
    }

    m_impl->selectedSet.erase(it);
    m_impl->selectedUuids.erase(
        std::remove(m_impl->selectedUuids.begin(), m_impl->selectedUuids.end(), key),
        m_impl->selectedUuids.end()
    );

    m_impl->NotifyChanged();
}

void SelectionModel::ClearSelection() {
    if (m_impl->selectedUuids.empty()) {
        return;
    }

    m_impl->selectedUuids.clear();
    m_impl->selectedSet.clear();
    m_impl->NotifyChanged();
}

void SelectionModel::SetSelectionRange(const std::vector<std::string>& uuids) {
    m_impl->selectedUuids.clear();
    m_impl->selectedSet.clear();

    for (const auto& id : uuids) {
        if (!id.empty() && m_impl->selectedSet.insert(id).second) {
            m_impl->selectedUuids.push_back(id);
        }
    }

    m_impl->NotifyChanged();
}

bool SelectionModel::IsSelected(compat::string_view nodeUuid) const noexcept {
    std::string key(nodeUuid.data(), nodeUuid.size());
    return m_impl->selectedSet.count(key) > 0;
}

const std::vector<std::string>& SelectionModel::GetSelectedUuids() const noexcept {
    return m_impl->selectedUuids;
}

compat::optional<std::string> SelectionModel::GetPrimarySelection() const noexcept {
    if (m_impl->selectedUuids.empty()) {
        return compat::nullopt;
    }
    return m_impl->selectedUuids.back();
}

size_t SelectionModel::GetSelectionCount() const noexcept {
    return m_impl->selectedUuids.size();
}

DesignerEvent<std::vector<std::string>>& SelectionModel::SelectionChangedSignal() noexcept {
    return m_impl->selectionChangedSignal;
}

} // namespace surface
} // namespace designer
} // namespace cppui
