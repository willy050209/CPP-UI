/**
 * @file DesignSurface.cpp
 * @brief 視覺化畫布設計表面主控制項實作
 */

#include "ui/designer/surface/DesignSurface.hpp"
#include "ui/core/LifetimeToken.hpp"
#include "ui/reactive/Signal.hpp"
#include "ui/designer/surface/DesignerHitTest.hpp"
#include "ui/designer/surface/SnapEngine.hpp"
#include "ui/designer/surface/AdornerOverlay.hpp"
#include "ui/designer/commands/WidgetCommands.hpp"
#include "ui/designer/commands/PropertyCommands.hpp"
#include "ui/designer/commands/MacroCommand.hpp"
#include "ui/text/Font.hpp"
#include <algorithm>
#include <cctype>

namespace cppui {
namespace designer {
namespace surface {

namespace {

struct ScrollLayout {
    bool hasH = false;
    bool hasV = false;
    float viewW = 0.0f;
    float viewH = 0.0f;
    float maxScrollX = 0.0f;
    float maxScrollY = 0.0f;
    core::Rect hTrackRect;
    core::Rect hThumbRect;
    core::Rect vTrackRect;
    core::Rect vThumbRect;
    core::Rect cornerRect;
};

static void DrawRectOutline(rendering::DrawingContext& dc, const core::Rect& r, core::Color color, float strokeWidth = 1.0f) {
    dc.DrawLine(core::Point(r.x, r.y), core::Point(r.x + r.width, r.y), color, strokeWidth);
    dc.DrawLine(core::Point(r.x + r.width, r.y), core::Point(r.x + r.width, r.y + r.height), color, strokeWidth);
    dc.DrawLine(core::Point(r.x + r.width, r.y + r.height), core::Point(r.x, r.y + r.height), color, strokeWidth);
    dc.DrawLine(core::Point(r.x, r.y + r.height), core::Point(r.x, r.y), color, strokeWidth);
}

} // anonymous namespace

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
    float fontScale = 1.0f;
    reactive::ScopedConnection selectionConn;

    // 2D 畫布滾動視口狀態
    float scrollX = 0.0f;
    float scrollY = 0.0f;
    core::Point scrollDragStart;
    float scrollDragStartOffset = 0.0f;
    static constexpr float SCROLLBAR_SIZE = 14.0f;

    core::Size CalculateContentExtent() const {
        float maxX = 800.0f;
        float maxY = 600.0f;
        if (document) {
            const auto& s = document->GetSettings();
            if (s.initialSize.width > 0.0f) maxX = std::max(maxX, s.initialSize.width);
            if (s.initialSize.height > 0.0f) maxY = std::max(maxY, s.initialSize.height);
            if (const auto* root = document->GetRoot()) {
                if (auto pw = root->GetProperty("Width")) {
                    if (auto val = pw->Get<double>()) maxX = std::max(maxX, static_cast<float>(*val));
                }
                if (auto ph = root->GetProperty("Height")) {
                    if (auto val = ph->Get<double>()) maxY = std::max(maxY, static_cast<float>(*val));
                }
            }
        }
        for (const auto& kv : boundsMap) {
            maxX = std::max(maxX, kv.second.x + kv.second.width + 40.0f);
            maxY = std::max(maxY, kv.second.y + kv.second.height + 40.0f);
        }
        return core::Size(maxX, maxY);
    }

    ScrollLayout GetScrollLayout(float curW, float curH) const {
        ScrollLayout sl;
        core::Size extent = CalculateContentExtent();
        sl.hasH = extent.width > curW;
        sl.hasV = extent.height > curH;
        if (sl.hasH && !sl.hasV && (extent.height > curH - SCROLLBAR_SIZE)) sl.hasV = true;
        if (sl.hasV && !sl.hasH && (extent.width > curW - SCROLLBAR_SIZE)) sl.hasH = true;

        sl.viewW = sl.hasV ? (curW - SCROLLBAR_SIZE) : curW;
        sl.viewH = sl.hasH ? (curH - SCROLLBAR_SIZE) : curH;

        sl.maxScrollX = std::max(0.0f, extent.width - sl.viewW);
        sl.maxScrollY = std::max(0.0f, extent.height - sl.viewH);

        if (sl.hasH) {
            sl.hTrackRect = core::Rect(0.0f, sl.viewH, sl.viewW, SCROLLBAR_SIZE);
            float thumbW = std::max(24.0f, sl.viewW * (sl.viewW / extent.width));
            float travel = sl.viewW - thumbW;
            float thumbX = sl.hTrackRect.x + (sl.maxScrollX > 0.0f ? (scrollX / sl.maxScrollX) * travel : 0.0f);
            sl.hThumbRect = core::Rect(thumbX, sl.viewH + 2.0f, thumbW, SCROLLBAR_SIZE - 4.0f);
        }

        if (sl.hasV) {
            sl.vTrackRect = core::Rect(sl.viewW, 0.0f, SCROLLBAR_SIZE, sl.viewH);
            float thumbH = std::max(24.0f, sl.viewH * (sl.viewH / extent.height));
            float travel = sl.viewH - thumbH;
            float thumbY = sl.vTrackRect.y + (sl.maxScrollY > 0.0f ? (scrollY / sl.maxScrollY) * travel : 0.0f);
            sl.vThumbRect = core::Rect(sl.viewW + 2.0f, thumbY, SCROLLBAR_SIZE - 4.0f, thumbH);
        }

        if (sl.hasH && sl.hasV) {
            sl.cornerRect = core::Rect(sl.viewW, sl.viewH, SCROLLBAR_SIZE, SCROLLBAR_SIZE);
        }

        return sl;
    }

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
    m_impl->selectionConn = m_impl->selectionModel.SelectionChangedSignal().Connect(GetLifetimeToken(), [this](const std::vector<std::string>&) {
        InvalidateVisual();
    });
    SetClipToBounds(true);
    SetFocusable(true);
}

DesignSurface::~DesignSurface() = default;

void DesignSurface::SetFontScale(float scale) {
    m_impl->fontScale = compat::clamp(scale, 0.5f, 5.0f);
    InvalidateVisual();
}

float DesignSurface::GetFontScale() const noexcept {
    return m_impl->fontScale;
}

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

core::Point DesignSurface::GetScrollOffset() const noexcept {
    return core::Point(m_impl->scrollX, m_impl->scrollY);
}

void DesignSurface::SetScrollOffset(float x, float y) {
    float curW = GetActualSize().width > 0.0f ? GetActualSize().width : GetWidth();
    float curH = GetActualSize().height > 0.0f ? GetActualSize().height : GetHeight();
    if (curW <= 0.0f) curW = 800.0f;
    if (curH <= 0.0f) curH = 520.0f;

    auto sl = m_impl->GetScrollLayout(curW, curH);
    float clampedX = compat::clamp(x, 0.0f, sl.maxScrollX);
    float clampedY = compat::clamp(y, 0.0f, sl.maxScrollY);

    if (m_impl->scrollX != clampedX || m_impl->scrollY != clampedY) {
        m_impl->scrollX = clampedX;
        m_impl->scrollY = clampedY;
        InvalidateVisual();
    }
}

core::Size DesignSurface::GetContentExtent() const noexcept {
    return m_impl->CalculateContentExtent();
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

std::string DesignSurface::AddNewWidget(NodeType type, const core::Point& position, const std::string& parentUuid) {
    if (!m_impl->document || !m_impl->historyManager) {
        return "";
    }

    WidgetNode* parent = nullptr;
    if (!parentUuid.empty()) {
        parent = m_impl->document->FindByUuid(parentUuid);
    }
    if (!parent) {
        parent = m_impl->document->GetRoot();
    }
    if (!parent) {
        return "";
    }

    // 自動產生唯一的標識符 (Unique Identifier)
    std::string typeName(NodeTypeName(type));
    std::string baseName = typeName;
    if (!baseName.empty()) {
        baseName[0] = static_cast<char>(std::tolower(static_cast<unsigned char>(baseName[0])));
    }
    std::string candidateName;
    int32_t counter = 1;
    while (true) {
        candidateName = baseName + std::to_string(counter++);
        if (!m_impl->document->FindByName(candidateName)) {
            break;
        }
    }

    auto newNode = std::make_unique<WidgetNode>(type, candidateName);
    std::string nodeUuid = newNode->GetUuid();

    // 依元件類型配置預設尺寸與文字
    float defaultWidth = 120.0f;
    float defaultHeight = 32.0f;

    switch (type) {
        case NodeType::Button:
            newNode->SetProperty("Content", PropertyValue(candidateName));
            newNode->SetProperty("Background", PropertyValue(core::Color(14, 110, 210)));
            newNode->SetProperty("Foreground", PropertyValue(core::Color(255, 255, 255)));
            newNode->SetProperty("FontSize", PropertyValue(13.0));
            newNode->SetProperty("FontFamily", PropertyValue(std::string("Segoe UI")));
            newNode->SetProperty("FontWeight", PropertyValue(int32_t(400)));
            break;
        case NodeType::TextBox:
            newNode->SetProperty("Text", PropertyValue(std::string("")));
            newNode->SetProperty("Placeholder", PropertyValue(candidateName));
            newNode->SetProperty("Background", PropertyValue(core::Color(36, 40, 50)));
            newNode->SetProperty("Foreground", PropertyValue(core::Color(240, 240, 240)));
            newNode->SetProperty("FontSize", PropertyValue(13.0));
            newNode->SetProperty("FontFamily", PropertyValue(std::string("Segoe UI")));
            newNode->SetProperty("FontWeight", PropertyValue(int32_t(400)));
            defaultWidth = 180.0f;
            break;
        case NodeType::Label:
        case NodeType::TextBlock:
            newNode->SetProperty("Content", PropertyValue(candidateName));
            newNode->SetProperty("FontSize", PropertyValue(13.0));
            newNode->SetProperty("FontFamily", PropertyValue(std::string("Segoe UI")));
            newNode->SetProperty("FontWeight", PropertyValue(int32_t(400)));
            newNode->SetProperty("Foreground", PropertyValue(core::Color(240, 240, 240)));
            defaultWidth = 120.0f;
            defaultHeight = 24.0f;
            break;
        case NodeType::CheckBox:
            newNode->SetProperty("Content", PropertyValue(candidateName));
            newNode->SetProperty("IsChecked", PropertyValue(false));
            newNode->SetProperty("FontSize", PropertyValue(13.0));
            newNode->SetProperty("FontFamily", PropertyValue(std::string("Segoe UI")));
            newNode->SetProperty("FontWeight", PropertyValue(int32_t(400)));
            newNode->SetProperty("Foreground", PropertyValue(core::Color(240, 240, 240)));
            defaultWidth = 160.0f;
            defaultHeight = 24.0f;
            break;
        case NodeType::RadioButton:
            newNode->SetProperty("Content", PropertyValue(candidateName));
            newNode->SetProperty("IsChecked", PropertyValue(false));
            newNode->SetProperty("FontSize", PropertyValue(13.0));
            newNode->SetProperty("FontFamily", PropertyValue(std::string("Segoe UI")));
            newNode->SetProperty("FontWeight", PropertyValue(int32_t(400)));
            newNode->SetProperty("Foreground", PropertyValue(core::Color(240, 240, 240)));
            defaultWidth = 160.0f;
            defaultHeight = 24.0f;
            break;
        case NodeType::ComboBox:
        case NodeType::ListBox:
            newNode->SetProperty("FontSize", PropertyValue(13.0));
            newNode->SetProperty("FontFamily", PropertyValue(std::string("Segoe UI")));
            newNode->SetProperty("FontWeight", PropertyValue(int32_t(400)));
            newNode->SetProperty("Foreground", PropertyValue(core::Color(240, 240, 240)));
            defaultWidth = 160.0f;
            defaultHeight = (type == NodeType::ListBox) ? 120.0f : 32.0f;
            break;
        case NodeType::DataGrid:
            defaultWidth = 320.0f;
            defaultHeight = 180.0f;
            break;
        case NodeType::Slider:
        case NodeType::ProgressBar:
            defaultWidth = 180.0f;
            defaultHeight = 24.0f;
            break;
        case NodeType::PictureBox:
            defaultWidth = 160.0f;
            defaultHeight = 120.0f;
            break;
        case NodeType::Grid:
        case NodeType::StackPanel:
        case NodeType::Canvas:
        case NodeType::Border:
        case NodeType::ScrollViewer:
            defaultWidth = 240.0f;
            defaultHeight = 180.0f;
            break;
        default:
            break;
    }

    newNode->SetProperty("Visibility", PropertyValue(layout::Visibility::Visible));

    newNode->SetProperty("Width", PropertyValue(static_cast<double>(defaultWidth)));
    newNode->SetProperty("Height", PropertyValue(static_cast<double>(defaultHeight)));
    newNode->SetAttachedProperty("Canvas.Left", PropertyValue(static_cast<double>(position.x)));
    newNode->SetAttachedProperty("Canvas.Top", PropertyValue(static_cast<double>(position.y)));

    core::Rect bounds(position.x, position.y, defaultWidth, defaultHeight);

    auto cmd = std::make_unique<AddWidgetCommand>(parent->GetUuid(), std::move(newNode));
    auto res = m_impl->historyManager->Execute(*m_impl->document, std::move(cmd));
    if (res.has_value()) {
        SetNodeBounds(nodeUuid, bounds);
        m_impl->selectionModel.SetSelection(nodeUuid);
        InvalidateVisual();
        return nodeUuid;
    }

    return "";
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
    float curW = GetActualSize().width > 0.0f ? GetActualSize().width : GetWidth();
    float curH = GetActualSize().height > 0.0f ? GetActualSize().height : GetHeight();
    if (curW <= 0.0f) curW = 800.0f;
    if (curH <= 0.0f) curH = 520.0f;

    auto sl = m_impl->GetScrollLayout(curW, curH);

    dc.PushClip(core::Rect(0.0f, 0.0f, sl.viewW, sl.viewH));
    dc.PushOffset(core::Point(-m_impl->scrollX, -m_impl->scrollY));

    core::Size extent = m_impl->CalculateContentExtent();

    if (m_impl->options.showGrid) {
        AdornerOverlay::RenderGrid(dc, extent, m_impl->options.gridSize, m_impl->options.gridColor);
    }

    // 繪製表單畫布邊界與背景
    core::Size formSize(800.0f, 600.0f);
    core::Color formBg(32, 34, 42);
    if (m_impl->document) {
        formSize = m_impl->document->GetSettings().initialSize;
        if (const auto* root = m_impl->document->GetRoot()) {
            if (auto p = root->GetProperty("Background")) {
                if (auto c = p->Get<core::Color>()) formBg = *c;
            }
        }
    }
    dc.DrawRoundedRectangle(core::Rect(0.0f, 0.0f, formSize.width, formSize.height), 0.0f, formBg);
    DrawRectOutline(dc, core::Rect(0.0f, 0.0f, formSize.width, formSize.height), core::Color(65, 70, 85), 1.0f);

    // 繪製畫布上的所有控制項 (AST Widgets Rendering)
    if (m_impl->document && m_impl->document->GetRoot()) {
        for (const auto& kv : m_impl->boundsMap) {
            const std::string& uuid = kv.first;
            core::Rect rect = kv.second;
            WidgetNode* node = m_impl->document->FindByUuid(uuid);
            if (!node || node->GetType() == NodeType::Grid || node->GetType() == NodeType::Canvas) {
                continue;
            }

            // 檢查 Visibility：Collapsed 或 Hidden 狀態以半透明呈現，便於視覺化編輯
            layout::Visibility vis = layout::Visibility::Visible;
            if (auto vp = node->GetProperty("Visibility")) {
                if (auto v = vp->Get<layout::Visibility>()) vis = *v;
                else if (auto s = vp->Get<std::string>()) {
                    if (*s == "Collapsed") vis = layout::Visibility::Collapsed;
                    else if (*s == "Hidden") vis = layout::Visibility::Hidden;
                }
            }
            bool isDimmed = (vis != layout::Visibility::Visible);
            if (isDimmed) {
                dc.PushOpacity(0.35f);
            }

            // 擷取背景色、前景色與字體樣式
            core::Color bg(14, 110, 210);
            if (node->GetType() == NodeType::TextBox) bg = core::Color(36, 40, 50);
            else if (node->GetType() == NodeType::ComboBox || node->GetType() == NodeType::ListBox) bg = core::Color(45, 50, 62);
            else if (node->GetType() == NodeType::Border) bg = core::Color(40, 44, 56);
            else if (node->GetType() == NodeType::Label || node->GetType() == NodeType::TextBlock) bg = core::Color(0, 0, 0, 0);

            if (auto bp = node->GetProperty("Background")) {
                if (auto c = bp->Get<core::Color>()) bg = *c;
            }

            core::Color fg(240, 240, 240);
            if (node->GetType() == NodeType::Button) fg = core::Color(255, 255, 255);
            if (auto fp = node->GetProperty("Foreground")) {
                if (auto c = fp->Get<core::Color>()) fg = *c;
            }

            float fontSize = 12.0f;
            if (auto fsp = node->GetProperty("FontSize")) {
                if (auto d = fsp->Get<double>()) fontSize = static_cast<float>(*d);
                else if (auto i = fsp->Get<int32_t>()) fontSize = static_cast<float>(*i);
            }

            std::string fontFamily = "Segoe UI";
            if (auto ffp = node->GetProperty("FontFamily")) {
                if (auto s = ffp->Get<std::string>()) fontFamily = *s;
            }

            text::FontWeight fontWeight = text::FontWeight::Normal;
            if (auto fwp = node->GetProperty("FontWeight")) {
                if (auto i = fwp->Get<int32_t>()) fontWeight = static_cast<text::FontWeight>(*i);
                else if (auto s = fwp->Get<std::string>()) {
                    if (*s == "Bold") fontWeight = text::FontWeight::Bold;
                    else if (*s == "SemiBold") fontWeight = text::FontWeight::SemiBold;
                    else if (*s == "Light") fontWeight = text::FontWeight::Light;
                    else if (*s == "Thin") fontWeight = text::FontWeight::Thin;
                    else if (*s == "ExtraBold") fontWeight = text::FontWeight::ExtraBold;
                    else if (*s == "Black") fontWeight = text::FontWeight::Black;
                }
            }

            // 繪製控制項幾何外觀
            if (node->GetType() == NodeType::Button) {
                dc.DrawRoundedRectangle(rect, 4.0f, bg);
            } else if (node->GetType() == NodeType::TextBox) {
                dc.DrawRoundedRectangle(rect, 3.0f, bg);
            } else if (node->GetType() == NodeType::CheckBox) {
                core::Rect chk(rect.x, rect.y + 4.0f, 16.0f, 16.0f);
                dc.DrawRoundedRectangle(chk, 2.0f, core::Color(24, 134, 75));
            } else if (node->GetType() == NodeType::RadioButton) {
                core::Rect rad(rect.x + 2.0f, rect.y + 4.0f, 14.0f, 14.0f);
                dc.DrawEllipse(rad, core::Color(0, 122, 204), 1.5f);
                dc.FillEllipse(core::Rect(rad.x + 3.0f, rad.y + 3.0f, 8.0f, 8.0f), core::Color(0, 122, 204));
            } else if (node->GetType() == NodeType::ComboBox) {
                dc.DrawRoundedRectangle(rect, 3.0f, bg);
                dc.DrawText("▾", core::Point(rect.x + rect.width - 18.0f, rect.y + 6.0f),
                            text::FontDescriptor("Segoe UI", 12.0f * m_impl->fontScale), core::Color(200, 200, 200));
            } else if (node->GetType() == NodeType::ListBox) {
                dc.DrawRoundedRectangle(rect, 3.0f, bg);
            } else if (node->GetType() == NodeType::Border) {
                float corner = 0.0f;
                if (auto cp = node->GetProperty("CornerRadius")) {
                    if (auto d = cp->Get<double>()) corner = static_cast<float>(*d);
                }
                if (bg.a > 0) dc.DrawRoundedRectangle(rect, corner, bg);
                core::Color borderBrush(70, 75, 90);
                if (auto bp = node->GetProperty("BorderBrush")) {
                    if (auto c = bp->Get<core::Color>()) borderBrush = *c;
                }
                DrawRectOutline(dc, rect, borderBrush, 1.0f);
            } else if (node->GetType() == NodeType::Slider) {
                float midY = rect.y + rect.height / 2.0f;
                dc.DrawLine(core::Point(rect.x + 6.0f, midY), core::Point(rect.x + rect.width - 6.0f, midY),
                            core::Color(70, 75, 90), 4.0f);
                core::Rect thumb(rect.x + rect.width * 0.4f - 6.0f, midY - 8.0f, 12.0f, 16.0f);
                dc.DrawRoundedRectangle(thumb, 2.0f, core::Color(0, 122, 255));
            } else if (node->GetType() == NodeType::ProgressBar) {
                dc.DrawRoundedRectangle(rect, 3.0f, core::Color(40, 45, 55));
                core::Rect fillRect(rect.x, rect.y, rect.width * 0.6f, rect.height);
                dc.DrawRoundedRectangle(fillRect, 3.0f, core::Color(24, 134, 75));
            } else if (node->GetType() == NodeType::PictureBox) {
                dc.DrawRoundedRectangle(rect, 4.0f, core::Color(32, 36, 44));
                dc.DrawText("🖼 [PictureBox]", core::Point(rect.x + 12.0f, rect.y + rect.height / 2.0f - 8.0f),
                            text::FontDescriptor("Segoe UI", 11.5f * m_impl->fontScale), core::Color(140, 145, 160));
            } else if (node->GetType() == NodeType::Separator) {
                float midY = rect.y + rect.height / 2.0f;
                dc.DrawLine(core::Point(rect.x, midY), core::Point(rect.x + rect.width, midY),
                            core::Color(70, 75, 90), 1.0f);
            } else if (node->GetType() == NodeType::Timer || node->GetType() == NodeType::MessageBox ||
                       node->GetType() == NodeType::OpenFileDialog || node->GetType() == NodeType::SaveFileDialog) {
                dc.DrawRoundedRectangle(rect, 4.0f, core::Color(45, 50, 62));
            }

            // 繪製控制項文字 (Content / Text / Placeholder)
            std::string text;
            if (auto cp = node->GetProperty("Content")) {
                if (auto s = cp->Get<std::string>()) text = *s;
            } else if (auto tp = node->GetProperty("Text")) {
                if (auto s = tp->Get<std::string>()) text = *s;
            }
            if (text.empty() && node->GetType() == NodeType::TextBox) {
                if (auto pp = node->GetProperty("Placeholder")) {
                    if (auto s = pp->Get<std::string>()) text = *s;
                }
            }

            if (!text.empty()) {
                core::Point origin(rect.x + 8.0f, rect.y + 6.0f);
                core::Color textColor = fg;
                if (node->GetType() == NodeType::TextBox && !node->GetProperty("Text")) {
                    textColor = core::Color(130, 135, 150);
                }
                if (node->GetType() == NodeType::CheckBox || node->GetType() == NodeType::RadioButton) {
                    origin.x = rect.x + 24.0f;
                    origin.y = rect.y + 4.0f;
                }
                text::FontDescriptor fontDesc(fontFamily, fontSize * m_impl->fontScale, fontWeight);
                dc.DrawText(text, origin, fontDesc, textColor);
            }

            if (isDimmed) {
                dc.PopOpacity();
            }
        }
    }

    dc.PopOffset();
    dc.PopClip();
}

void DesignSurface::OnPostRender(rendering::DrawingContext& dc) {
    float curW = GetActualSize().width > 0.0f ? GetActualSize().width : GetWidth();
    float curH = GetActualSize().height > 0.0f ? GetActualSize().height : GetHeight();
    if (curW <= 0.0f) curW = 800.0f;
    if (curH <= 0.0f) curH = 520.0f;

    auto sl = m_impl->GetScrollLayout(curW, curH);

    // 1-3. 在世界坐標系繪製飾條、選取框與導引線
    dc.PushClip(core::Rect(0.0f, 0.0f, sl.viewW, sl.viewH));
    dc.PushOffset(core::Point(-m_impl->scrollX, -m_impl->scrollY));

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

    dc.PopOffset();
    dc.PopClip();

    // 4. 繪製水平與垂直卷軸 (在視口座標系)
    dc.PushClip(core::Rect(0.0f, 0.0f, curW, curH));
    if (sl.hasH) {
        dc.DrawRoundedRectangle(sl.hTrackRect, 0.0f, core::Color(32, 35, 44));
        core::Color thumbColor = (m_impl->dragMode == DragMode::ScrollH) ? core::Color(110, 115, 130) : core::Color(70, 75, 90);
        dc.DrawRoundedRectangle(sl.hThumbRect, 4.0f, thumbColor);
    }
    if (sl.hasV) {
        dc.DrawRoundedRectangle(sl.vTrackRect, 0.0f, core::Color(32, 35, 44));
        core::Color thumbColor = (m_impl->dragMode == DragMode::ScrollV) ? core::Color(110, 115, 130) : core::Color(70, 75, 90);
        dc.DrawRoundedRectangle(sl.vThumbRect, 4.0f, thumbColor);
    }
    if (sl.hasH && sl.hasV) {
        dc.DrawRoundedRectangle(sl.cornerRect, 0.0f, core::Color(26, 28, 36));
    }
    dc.PopClip();
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

void DesignSurface::OnPointerWheel(input::PointerEventArgs& e) {
    OnPointerWheelCore(e);
}

void DesignSurface::OnPointerPressedCore(input::PointerEventArgs& e) {
    core::Point pt = e.GetPosition(this);
    float curW = GetActualSize().width > 0.0f ? GetActualSize().width : GetWidth();
    float curH = GetActualSize().height > 0.0f ? GetActualSize().height : GetHeight();
    if (curW <= 0.0f) curW = 800.0f;
    if (curH <= 0.0f) curH = 520.0f;
    auto sl = m_impl->GetScrollLayout(curW, curH);

    if (e.IsLeftButton()) {
        // 0. 優先檢驗卷軸點擊
        if (sl.hasH && sl.hTrackRect.Contains(pt)) {
            if (sl.hThumbRect.Contains(pt)) {
                m_impl->dragMode = DragMode::ScrollH;
                m_impl->scrollDragStart = pt;
                m_impl->scrollDragStartOffset = m_impl->scrollX;
            } else {
                if (pt.x < sl.hThumbRect.x) {
                    SetScrollOffset(m_impl->scrollX - sl.viewW * 0.8f, m_impl->scrollY);
                } else {
                    SetScrollOffset(m_impl->scrollX + sl.viewW * 0.8f, m_impl->scrollY);
                }
            }
            e.SetHandled(true);
            return;
        }

        if (sl.hasV && sl.vTrackRect.Contains(pt)) {
            if (sl.vThumbRect.Contains(pt)) {
                m_impl->dragMode = DragMode::ScrollV;
                m_impl->scrollDragStart = pt;
                m_impl->scrollDragStartOffset = m_impl->scrollY;
            } else {
                if (pt.y < sl.vThumbRect.y) {
                    SetScrollOffset(m_impl->scrollX, m_impl->scrollY - sl.viewH * 0.8f);
                } else {
                    SetScrollOffset(m_impl->scrollX, m_impl->scrollY + sl.viewH * 0.8f);
                }
            }
            e.SetHandled(true);
            return;
        }

        if (sl.hasH && sl.hasV && sl.cornerRect.Contains(pt)) {
            e.SetHandled(true);
            return;
        }

        core::Point worldPt(pt.x + m_impl->scrollX, pt.y + m_impl->scrollY);

        // 1. 檢驗是否點擊主選取元件之 8 向 Handle
        if (auto prim = m_impl->selectionModel.GetPrimarySelection()) {
            core::Rect primBounds = GetNodeBounds(*prim);
            HandleType h = DesignerHitTest::HitTestHandle(primBounds, worldPt, m_impl->options.handleSize);
            if (h != HandleType::None) {
                m_impl->dragMode = DragMode::Resize;
                m_impl->activeHandle = h;
                m_impl->dragStartPos = worldPt;
                m_impl->dragInitialBounds = primBounds;
                m_impl->currentDragBounds = primBounds;
                m_impl->draggedNodeUuid = *prim;
                e.SetHandled(true);
                return;
            }
        }

        // 2. 檢驗是否命中元件節點
        WidgetNode* root = m_impl->document ? m_impl->document->GetRoot() : nullptr;
        WidgetNode* hit = DesignerHitTest::HitTestNode(root, m_impl->boundsMap, worldPt);
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
            m_impl->dragStartPos = worldPt;
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
        m_impl->dragStartPos = worldPt;
        m_impl->rubberBandRect = core::Rect(worldPt.x, worldPt.y, 0.0f, 0.0f);
        e.SetHandled(true);
    }
}

void DesignSurface::OnPointerMovedCore(input::PointerEventArgs& e) {
    core::Point pt = e.GetPosition(this);
    float curW = GetActualSize().width > 0.0f ? GetActualSize().width : GetWidth();
    float curH = GetActualSize().height > 0.0f ? GetActualSize().height : GetHeight();
    if (curW <= 0.0f) curW = 800.0f;
    if (curH <= 0.0f) curH = 520.0f;
    auto sl = m_impl->GetScrollLayout(curW, curH);

    // 卷軸拖曳處理
    if (m_impl->dragMode == DragMode::ScrollH) {
        float deltaX = pt.x - m_impl->scrollDragStart.x;
        float travel = sl.viewW - sl.hThumbRect.width;
        if (travel > 0.0f && sl.maxScrollX > 0.0f) {
            float deltaOffset = (deltaX / travel) * sl.maxScrollX;
            SetScrollOffset(m_impl->scrollDragStartOffset + deltaOffset, m_impl->scrollY);
        }
        e.SetHandled(true);
        return;
    }

    if (m_impl->dragMode == DragMode::ScrollV) {
        float deltaY = pt.y - m_impl->scrollDragStart.y;
        float travel = sl.viewH - sl.vThumbRect.height;
        if (travel > 0.0f && sl.maxScrollY > 0.0f) {
            float deltaOffset = (deltaY / travel) * sl.maxScrollY;
            SetScrollOffset(m_impl->scrollX, m_impl->scrollDragStartOffset + deltaOffset);
        }
        e.SetHandled(true);
        return;
    }

    core::Point worldPt(pt.x + m_impl->scrollX, pt.y + m_impl->scrollY);

    if (m_impl->dragMode == DragMode::Move) {
        float dx = worldPt.x - m_impl->dragStartPos.x;
        float dy = worldPt.y - m_impl->dragStartPos.y;

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
        float dx = worldPt.x - m_impl->dragStartPos.x;
        float dy = worldPt.y - m_impl->dragStartPos.y;

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
        float minX = std::min(m_impl->dragStartPos.x, worldPt.x);
        float minY = std::min(m_impl->dragStartPos.y, worldPt.y);
        float maxX = std::max(m_impl->dragStartPos.x, worldPt.x);
        float maxY = std::max(m_impl->dragStartPos.y, worldPt.y);

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

    // 閒置滑鼠懸停：檢查是否在卷軸上或 Handle 上
    if ((sl.hasH && sl.hTrackRect.Contains(pt)) || (sl.hasV && sl.vTrackRect.Contains(pt))) {
        SetCursor(controls::CursorType::Default);
        return;
    }

    if (auto prim = m_impl->selectionModel.GetPrimarySelection()) {
        core::Rect primBounds = GetNodeBounds(*prim);
        HandleType h = DesignerHitTest::HitTestHandle(primBounds, worldPt, m_impl->options.handleSize);
        SetCursor(GetCursorForHandle(h));
    } else {
        SetCursor(controls::CursorType::Default);
    }
}

void DesignSurface::OnPointerReleasedCore(input::PointerEventArgs& e) {
    if (m_impl->dragMode == DragMode::ScrollH || m_impl->dragMode == DragMode::ScrollV) {
        m_impl->dragMode = DragMode::None;
        InvalidateVisual();
        e.SetHandled(true);
        return;
    }

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

void DesignSurface::OnPointerWheelCore(input::PointerEventArgs& e) {
    float step = 48.0f;
    if (e.GetModifiers() == pal::KeyModifiers::Shift) {
        float dx = (e.GetWheelDeltaY() > 0 ? -step : step);
        SetScrollOffset(m_impl->scrollX + dx, m_impl->scrollY);
    } else {
        float dy = (e.GetWheelDeltaY() > 0 ? -step : (e.GetWheelDeltaY() < 0 ? step : 0.0f));
        float dx = (e.GetWheelDeltaX() > 0 ? step : (e.GetWheelDeltaX() < 0 ? -step : 0.0f));
        SetScrollOffset(m_impl->scrollX + dx, m_impl->scrollY + dy);
    }
    e.SetHandled(true);
}

} // namespace surface
} // namespace designer
} // namespace cppui
