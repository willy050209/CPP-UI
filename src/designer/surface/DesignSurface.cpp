/**
 * @file DesignSurface.cpp
 * @brief 視覺化畫布設計表面主控制項實作
 */

#include "ui/designer/surface/DesignSurface.hpp"
#include "ui/designer/surface/DesignerHitTest.hpp"
#include "ui/designer/surface/SnapEngine.hpp"
#include "ui/designer/surface/AdornerOverlay.hpp"
#include "ui/designer/commands/WidgetCommands.hpp"
#include "ui/designer/commands/PropertyCommands.hpp"
#include "ui/designer/commands/MacroCommand.hpp"
#include <algorithm>

namespace cppui {
namespace designer {
namespace surface {

struct DesignSurface::Impl {
    FormDocumentNode* document = nullptr;
    CommandHistoryManager* historyManager = nullptr;
    SelectionModel selectionModel;
    SurfaceOptions options;
    std::map<std::string, core::Rect> boundsMap;

    // 拖曳狀態機
    DragMode dragMode = DragMode::None;
    HandleType activeHandle = HandleType::None;
    core::Point dragStartPos;
    core::Rect dragInitialBounds;
    core::Rect currentDragBounds;
    core::Rect rubberBandRect;
    std::vector<SnapLine> activeGuides;
    std::string draggedNodeUuid;

    void SyncBoundsFromDocument() {
        if (!document) return;
        for (auto& kv : boundsMap) {
            if (WidgetNode* node = document->FindByUuid(kv.first)) {
                auto left = node->GetAttachedProperty("Canvas.Left");
                auto top = node->GetAttachedProperty("Canvas.Top");
                auto w = node->GetProperty("Width");
                auto h = node->GetProperty("Height");
                if (left.has_value() && left->Is<double>()) kv.second.x = static_cast<float>(left->Get<double>().value());
                if (top.has_value() && top->Is<double>()) kv.second.y = static_cast<float>(top->Get<double>().value());
                if (w.has_value() && w->Is<double>()) kv.second.width = static_cast<float>(w->Get<double>().value());
                if (h.has_value() && h->Is<double>()) kv.second.height = static_cast<float>(h->Get<double>().value());
            }
        }
    }
};

DesignSurface::DesignSurface(
    FormDocumentNode* document,
    CommandHistoryManager* historyManager
) : m_impl(std::make_unique<Impl>()) {
    m_impl->document = document;
    m_impl->historyManager = historyManager;
    SetFocusable(true);
}

DesignSurface::~DesignSurface() = default;

void DesignSurface::SetDocument(FormDocumentNode* document) {
    m_impl->document = document;
    m_impl->selectionModel.ClearSelection();
    m_impl->boundsMap.clear();
}

FormDocumentNode* DesignSurface::GetDocument() const noexcept {
    return m_impl->document;
}

void DesignSurface::SetHistoryManager(CommandHistoryManager* historyManager) {
    m_impl->historyManager = historyManager;
}

CommandHistoryManager* DesignSurface::GetHistoryManager() const noexcept {
    return m_impl->historyManager;
}

SelectionModel& DesignSurface::GetSelectionModel() noexcept {
    return m_impl->selectionModel;
}

const SelectionModel& DesignSurface::GetSelectionModel() const noexcept {
    return m_impl->selectionModel;
}

SurfaceOptions& DesignSurface::GetOptions() noexcept {
    return m_impl->options;
}

const SurfaceOptions& DesignSurface::GetOptions() const noexcept {
    return m_impl->options;
}

void DesignSurface::SetOptions(const SurfaceOptions& options) {
    m_impl->options = options;
    InvalidateVisual();
}

void DesignSurface::SetNodeBounds(const std::string& nodeUuid, const core::Rect& bounds) {
    m_impl->boundsMap[nodeUuid] = bounds;
}

core::Rect DesignSurface::GetNodeBounds(const std::string& nodeUuid) const {
    auto it = m_impl->boundsMap.find(nodeUuid);
    if (it != m_impl->boundsMap.end()) {
        return it->second;
    }
    if (m_impl->document) {
        if (const WidgetNode* n = m_impl->document->FindByUuid(nodeUuid)) {
            return DesignerHitTest::EstimateNodeBounds(*n);
        }
    }
    return core::Rect(0.0f, 0.0f, 100.0f, 30.0f);
}

const std::map<std::string, core::Rect>& DesignSurface::GetAllNodeBounds() const noexcept {
    return m_impl->boundsMap;
}

bool DesignSurface::DeleteSelected() {
    if (!m_impl->document || !m_impl->historyManager) {
        return false;
    }

    auto uuids = m_impl->selectionModel.GetSelectedUuids();
    if (uuids.empty()) {
        return false;
    }

    bool anyDeleted = false;
    for (const auto& uuid : uuids) {
        WidgetNode* node = m_impl->document->FindByUuid(uuid);
        if (node && node->GetParent()) {
            std::string parentUuid = node->GetParent()->GetUuid();
            auto cmd = std::make_unique<RemoveWidgetCommand>(parentUuid, uuid);
            auto res = m_impl->historyManager->Execute(*m_impl->document, std::move(cmd));
            if (res.has_value()) {
                m_impl->boundsMap.erase(uuid);
                anyDeleted = true;
            }
        }
    }

    m_impl->selectionModel.ClearSelection();
    return anyDeleted;
}

bool DesignSurface::Undo() {
    if (m_impl->historyManager && m_impl->document && m_impl->historyManager->CanUndo()) {
        auto res = m_impl->historyManager->Undo(*m_impl->document);
        if (res.has_value()) {
            m_impl->SyncBoundsFromDocument();
            InvalidateVisual();
            return true;
        }
    }
    return false;
}

bool DesignSurface::Redo() {
    if (m_impl->historyManager && m_impl->document && m_impl->historyManager->CanRedo()) {
        auto res = m_impl->historyManager->Redo(*m_impl->document);
        if (res.has_value()) {
            m_impl->SyncBoundsFromDocument();
            InvalidateVisual();
            return true;
        }
    }
    return false;
}

DragMode DesignSurface::GetCurrentDragMode() const noexcept {
    return m_impl->dragMode;
}

HandleType DesignSurface::GetActiveHandle() const noexcept {
    return m_impl->activeHandle;
}

const std::vector<SnapLine>& DesignSurface::GetActiveGuides() const noexcept {
    return m_impl->activeGuides;
}

core::Rect DesignSurface::GetRubberBandRect() const noexcept {
    return m_impl->rubberBandRect;
}

void DesignSurface::OnRender(rendering::DrawingContext& dc) {
    if (m_impl->options.showGrid) {
        core::Size sz(GetWidth(), GetHeight());
        if (sz.width <= 0.0f) sz.width = 1920.0f;
        if (sz.height <= 0.0f) sz.height = 1080.0f;
        AdornerOverlay::RenderGrid(dc, sz, m_impl->options.gridSize, m_impl->options.gridColor);
    }
}

void DesignSurface::OnPostRender(rendering::DrawingContext& dc) {
    // 1. 繪製所有被選取節點的飾條與 Handle
    auto primary = m_impl->selectionModel.GetPrimarySelection();
    for (const auto& uuid : m_impl->selectionModel.GetSelectedUuids()) {
        core::Rect r = GetNodeBounds(uuid);
        bool isPrimary = (primary.has_value() && *primary == uuid);
        AdornerOverlay::RenderSelection(dc, r, m_impl->options, isPrimary);
    }

    // 2. 繪製框選外框 (Rubber-band)
    if (m_impl->dragMode == DragMode::RubberBand && !m_impl->rubberBandRect.IsEmpty()) {
        AdornerOverlay::RenderRubberBand(
            dc,
            m_impl->rubberBandRect,
            m_impl->options.selectionBorderColor,
            m_impl->options.selectionFillColor
        );
    }

    // 3. 繪製吸附導引線
    if (!m_impl->activeGuides.empty()) {
        AdornerOverlay::RenderSnapGuides(dc, m_impl->activeGuides);
    }
}

void DesignSurface::OnPointerPressed(input::PointerEventArgs& e) {
    OnPointerPressedCore(e);
}

void DesignSurface::OnPointerMoved(input::PointerEventArgs& e) {
    OnPointerMovedCore(e);
}

void DesignSurface::OnPointerReleased(input::PointerEventArgs& e) {
    OnPointerReleasedCore(e);
}

void DesignSurface::OnPointerPressedCore(input::PointerEventArgs& e) {
    core::Point pt = e.GetPosition(this);

    if (e.IsLeftButton()) {
        // 1. 優先檢驗是否點擊主選取元件之 8 向 Handle
        if (auto prim = m_impl->selectionModel.GetPrimarySelection()) {
            core::Rect primBounds = GetNodeBounds(*prim);
            HandleType h = DesignerHitTest::HitTestHandle(primBounds, pt, m_impl->options.handleSize);
            if (h != HandleType::None) {
                m_impl->dragMode = DragMode::Resize;
                m_impl->activeHandle = h;
                m_impl->dragStartPos = pt;
                m_impl->dragInitialBounds = primBounds;
                m_impl->currentDragBounds = primBounds;
                m_impl->draggedNodeUuid = *prim;
                e.SetHandled(true);
                return;
            }
        }

        // 2. 檢驗是否命中元件節點
        WidgetNode* root = m_impl->document ? m_impl->document->GetRoot() : nullptr;
        WidgetNode* hit = DesignerHitTest::HitTestNode(root, m_impl->boundsMap, pt);
        if (hit) {
            std::string hitUuid = hit->GetUuid();
            bool isMultiKey = (e.GetModifiers() == pal::KeyModifiers::Shift || e.GetModifiers() == pal::KeyModifiers::Control);

            if (isMultiKey) {
                m_impl->selectionModel.AddToSelection(hitUuid);
            } else if (!m_impl->selectionModel.IsSelected(hitUuid)) {
                m_impl->selectionModel.SetSelection(hitUuid);
            }

            m_impl->dragMode = DragMode::Move;
            m_impl->draggedNodeUuid = hitUuid;
            m_impl->dragStartPos = pt;
            m_impl->dragInitialBounds = GetNodeBounds(hitUuid);
            m_impl->currentDragBounds = m_impl->dragInitialBounds;
            e.SetHandled(true);
            return;
        }

        // 3. 點擊畫布空白處：清空選取或啟動框選
        bool isMultiKey = (e.GetModifiers() == pal::KeyModifiers::Shift || e.GetModifiers() == pal::KeyModifiers::Control);
        if (!isMultiKey) {
            m_impl->selectionModel.ClearSelection();
        }

        m_impl->dragMode = DragMode::RubberBand;
        m_impl->dragStartPos = pt;
        m_impl->rubberBandRect = core::Rect(pt.x, pt.y, 0.0f, 0.0f);
        e.SetHandled(true);
    }
}

void DesignSurface::OnPointerMovedCore(input::PointerEventArgs& e) {
    core::Point pt = e.GetPosition(this);

    if (m_impl->dragMode == DragMode::Move) {
        float dx = pt.x - m_impl->dragStartPos.x;
        float dy = pt.y - m_impl->dragStartPos.y;

        core::Rect targetRect(
            m_impl->dragInitialBounds.x + dx,
            m_impl->dragInitialBounds.y + dy,
            m_impl->dragInitialBounds.width,
            m_impl->dragInitialBounds.height
        );

        // 收集其他兄弟節點邊界
        std::vector<core::Rect> siblings;
        for (const auto& kv : m_impl->boundsMap) {
            if (kv.first != m_impl->draggedNodeUuid) {
                siblings.push_back(kv.second);
            }
        }

        SnapResult snap = SnapEngine::CalculateSnap(targetRect, siblings, m_impl->options);
        m_impl->currentDragBounds.x = snap.snappedPos.x;
        m_impl->currentDragBounds.y = snap.snappedPos.y;
        m_impl->currentDragBounds.width = targetRect.width;
        m_impl->currentDragBounds.height = targetRect.height;
        m_impl->activeGuides = std::move(snap.activeGuides);

        SetNodeBounds(m_impl->draggedNodeUuid, m_impl->currentDragBounds);

        // 即時反映至 AST 節點 (不記錄至 history，待 Released 時透過 MacroCommand 提交)
        if (m_impl->document) {
            if (WidgetNode* node = m_impl->document->FindByUuid(m_impl->draggedNodeUuid)) {
                node->SetAttachedProperty("Canvas.Left", static_cast<double>(m_impl->currentDragBounds.x));
                node->SetAttachedProperty("Canvas.Top", static_cast<double>(m_impl->currentDragBounds.y));
            }
        }

        InvalidateVisual();
        e.SetHandled(true);
        return;
    }

    if (m_impl->dragMode == DragMode::Resize) {
        float dx = pt.x - m_impl->dragStartPos.x;
        float dy = pt.y - m_impl->dragStartPos.y;

        core::Rect r = m_impl->dragInitialBounds;

        switch (m_impl->activeHandle) {
            case HandleType::TopLeft:
                r.x += dx; r.y += dy; r.width -= dx; r.height -= dy; break;
            case HandleType::TopCenter:
                r.y += dy; r.height -= dy; break;
            case HandleType::TopRight:
                r.y += dy; r.width += dx; r.height -= dy; break;
            case HandleType::MiddleLeft:
                r.x += dx; r.width -= dx; break;
            case HandleType::MiddleRight:
                r.width += dx; break;
            case HandleType::BottomLeft:
                r.x += dx; r.width -= dx; r.height += dy; break;
            case HandleType::BottomCenter:
                r.height += dy; break;
            case HandleType::BottomRight:
                r.width += dx; r.height += dy; break;
            default: break;
        }

        if (r.width < 10.0f) r.width = 10.0f;
        if (r.height < 10.0f) r.height = 10.0f;

        m_impl->currentDragBounds = r;
        SetNodeBounds(m_impl->draggedNodeUuid, m_impl->currentDragBounds);

        // 即時反映至 AST 節點
        if (m_impl->document) {
            if (WidgetNode* node = m_impl->document->FindByUuid(m_impl->draggedNodeUuid)) {
                node->SetProperty("Width", static_cast<double>(r.width));
                node->SetProperty("Height", static_cast<double>(r.height));
            }
        }

        InvalidateVisual();
        e.SetHandled(true);
        return;
    }

    if (m_impl->dragMode == DragMode::RubberBand) {
        float minX = std::min(m_impl->dragStartPos.x, pt.x);
        float minY = std::min(m_impl->dragStartPos.y, pt.y);
        float maxX = std::max(m_impl->dragStartPos.x, pt.x);
        float maxY = std::max(m_impl->dragStartPos.y, pt.y);

        m_impl->rubberBandRect = core::Rect(minX, minY, maxX - minX, maxY - minY);

        std::vector<std::string> hitUuids;
        for (const auto& kv : m_impl->boundsMap) {
            if (!m_impl->rubberBandRect.Intersect(kv.second).IsEmpty()) {
                hitUuids.push_back(kv.first);
            }
        }
        m_impl->selectionModel.SetSelectionRange(hitUuids);

        InvalidateVisual();
        e.SetHandled(true);
        return;
    }

    // 閒置滑鼠懸停：檢查是否懸停於 Handle 上，即時切換游標樣式
    if (auto prim = m_impl->selectionModel.GetPrimarySelection()) {
        core::Rect primBounds = GetNodeBounds(*prim);
        HandleType h = DesignerHitTest::HitTestHandle(primBounds, pt, m_impl->options.handleSize);
        SetCursor(GetCursorForHandle(h));
    } else {
        SetCursor(controls::CursorType::Default);
    }
}

void DesignSurface::OnPointerReleasedCore(input::PointerEventArgs& e) {
    if (m_impl->dragMode == DragMode::Move) {
        if (m_impl->document && m_impl->historyManager && !m_impl->draggedNodeUuid.empty()) {
            if (m_impl->currentDragBounds.x != m_impl->dragInitialBounds.x ||
                m_impl->currentDragBounds.y != m_impl->dragInitialBounds.y) {
                // 先將 AST 節點數值還原為拖曳前數值，使 MacroCommand 執行時能正確補捉到 m_oldValue
                if (WidgetNode* node = m_impl->document->FindByUuid(m_impl->draggedNodeUuid)) {
                    node->SetAttachedProperty("Canvas.Left", static_cast<double>(m_impl->dragInitialBounds.x));
                    node->SetAttachedProperty("Canvas.Top", static_cast<double>(m_impl->dragInitialBounds.y));
                }

                auto macro = std::make_unique<MacroCommand>("Move Widget");
                macro->AddCommand(std::make_unique<SetAttachedPropertyCommand>(
                    m_impl->draggedNodeUuid,
                    "Canvas.Left",
                    static_cast<double>(m_impl->currentDragBounds.x)
                ));
                macro->AddCommand(std::make_unique<SetAttachedPropertyCommand>(
                    m_impl->draggedNodeUuid,
                    "Canvas.Top",
                    static_cast<double>(m_impl->currentDragBounds.y)
                ));
                (void)m_impl->historyManager->Execute(*m_impl->document, std::move(macro));
            }
        }
    } else if (m_impl->dragMode == DragMode::Resize) {
        if (m_impl->document && m_impl->historyManager && !m_impl->draggedNodeUuid.empty()) {
            if (m_impl->currentDragBounds.width != m_impl->dragInitialBounds.width ||
                m_impl->currentDragBounds.height != m_impl->dragInitialBounds.height) {
                if (WidgetNode* node = m_impl->document->FindByUuid(m_impl->draggedNodeUuid)) {
                    node->SetProperty("Width", static_cast<double>(m_impl->dragInitialBounds.width));
                    node->SetProperty("Height", static_cast<double>(m_impl->dragInitialBounds.height));
                }

                auto macro = std::make_unique<MacroCommand>("Resize Widget");
                macro->AddCommand(std::make_unique<SetPropertyCommand>(
                    m_impl->draggedNodeUuid,
                    "Width",
                    static_cast<double>(m_impl->currentDragBounds.width)
                ));
                macro->AddCommand(std::make_unique<SetPropertyCommand>(
                    m_impl->draggedNodeUuid,
                    "Height",
                    static_cast<double>(m_impl->currentDragBounds.height)
                ));
                (void)m_impl->historyManager->Execute(*m_impl->document, std::move(macro));
            }
        }
    }

    m_impl->dragMode = DragMode::None;
    m_impl->activeHandle = HandleType::None;
    m_impl->activeGuides.clear();
    m_impl->rubberBandRect = core::Rect();
    SetCursor(controls::CursorType::Default);
    InvalidateVisual();
    e.SetHandled(true);
}

} // namespace surface
} // namespace designer
} // namespace cppui
