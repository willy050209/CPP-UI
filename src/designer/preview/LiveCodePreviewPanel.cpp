/**
 * @file LiveCodePreviewPanel.cpp
 * @brief CPPUI Designer 即時代碼預覽面板實作
 */

#include "ui/designer/preview/LiveCodePreviewPanel.hpp"
#include "ui/designer/codegen/FormCodegenPipeline.hpp"
#include "ui/core/LifetimeToken.hpp"
#include "ui/reactive/Signal.hpp"

namespace cppui {
namespace designer {
namespace preview {

struct LiveCodePreviewPanel::Impl {
    FormDocumentNode* document = nullptr;
    surface::SelectionModel* selectionModel = nullptr;
    PreviewViewMode viewMode = PreviewViewMode::FluentDsl;

    codegen::FormCodegenPipeline pipeline;
    codegen::CodegenResult cachedResult;
    std::string activeText;

    DesignerEvent<std::pair<PreviewViewMode, std::string>> codeUpdatedSignal;

    core::LifetimeToken lifetimeToken{ core::LifetimeToken::Create() };

    reactive::ScopedConnection propConn;
    reactive::ScopedConnection nodeAddConn;
    reactive::ScopedConnection nodeRemConn;
    reactive::ScopedConnection nodeMovConn;
    reactive::ScopedConnection settingsConn;

    ~Impl() {
        DisconnectDocumentSignals();
        lifetimeToken.Invalidate();
    }

    void DisconnectDocumentSignals() {
        propConn.Disconnect();
        nodeAddConn.Disconnect();
        nodeRemConn.Disconnect();
        nodeMovConn.Disconnect();
        settingsConn.Disconnect();
    }

    void ConnectDocumentSignals() {
        DisconnectDocumentSignals();

        if (document) {
            propConn = document->PropertyChangedSignal().Connect(lifetimeToken, [this](const PropertyChangedPayload&) {
                RefreshCode();
            });
            nodeAddConn = document->NodeAddedSignal().Connect(lifetimeToken, [this](const NodeAddedPayload&) {
                RefreshCode();
            });
            nodeRemConn = document->NodeRemovedSignal().Connect(lifetimeToken, [this](const NodeRemovedPayload&) {
                RefreshCode();
            });
            nodeMovConn = document->NodeMovedSignal().Connect(lifetimeToken, [this](const NodeMovedPayload&) {
                RefreshCode();
            });
            settingsConn = document->SettingsChangedSignal().Connect(lifetimeToken, [this](const FormSettingsChangedPayload&) {
                RefreshCode();
            });
        }
    }

    void RefreshCode() {
        if (!document) {
            cachedResult = codegen::CodegenResult{};
            activeText.clear();
            codeUpdatedSignal.Notify({ viewMode, activeText });
            return;
        }

        codegen::CodegenOptions opts;
        opts.rootNamespace = document->GetSettings().targetNamespace;
        opts.formClassName = document->GetSettings().className;

        auto res = pipeline.Generate(*document, opts);
        if (res.has_value()) {
            cachedResult = std::move(*res);
            UpdateActiveText();
            codeUpdatedSignal.Notify({ viewMode, activeText });
        }
    }

    void UpdateActiveText() {
        switch (viewMode) {
            case PreviewViewMode::FluentDsl:
                activeText = cachedResult.fluentDslSnippet;
                break;
            case PreviewViewMode::ClassicDesignerHpp:
                activeText = cachedResult.designerHeader;
                break;
            case PreviewViewMode::ClassicHeader:
                activeText = cachedResult.userHeader;
                break;
            case PreviewViewMode::ClassicSource:
                activeText = cachedResult.userSource;
                break;
            case PreviewViewMode::CMakeLists:
                activeText = cachedResult.cmakeSnippet;
                break;
        }
    }
};

LiveCodePreviewPanel::LiveCodePreviewPanel(
    FormDocumentNode* document,
    surface::SelectionModel* selectionModel
) : m_impl(std::make_unique<Impl>()) {
    m_impl->document = document;
    m_impl->selectionModel = selectionModel;
    m_impl->ConnectDocumentSignals();
    m_impl->RefreshCode();
}

LiveCodePreviewPanel::~LiveCodePreviewPanel() = default;

void LiveCodePreviewPanel::SetDocument(FormDocumentNode* document) {
    if (m_impl->document != document) {
        m_impl->document = document;
        m_impl->ConnectDocumentSignals();
        m_impl->RefreshCode();
    }
}

FormDocumentNode* LiveCodePreviewPanel::GetDocument() const noexcept {
    return m_impl->document;
}

void LiveCodePreviewPanel::SetSelectionModel(surface::SelectionModel* selectionModel) {
    m_impl->selectionModel = selectionModel;
}

surface::SelectionModel* LiveCodePreviewPanel::GetSelectionModel() const noexcept {
    return m_impl->selectionModel;
}

void LiveCodePreviewPanel::SetViewMode(PreviewViewMode mode) {
    if (m_impl->viewMode != mode) {
        m_impl->viewMode = mode;
        m_impl->UpdateActiveText();
        m_impl->codeUpdatedSignal.Notify({ mode, m_impl->activeText });
    }
}

PreviewViewMode LiveCodePreviewPanel::GetViewMode() const noexcept {
    return m_impl->viewMode;
}

const std::string& LiveCodePreviewPanel::GetActiveCodeText() const noexcept {
    return m_impl->activeText;
}

void LiveCodePreviewPanel::Refresh() {
    m_impl->RefreshCode();
}

compat::optional<codegen::SourceSpan> LiveCodePreviewPanel::GetHighlightedSpan() const {
    if (!m_impl->selectionModel) {
        return compat::nullopt;
    }
    auto primary = m_impl->selectionModel->GetPrimarySelection();
    if (!primary.has_value()) {
        return compat::nullopt;
    }
    return GetSpanForNode(*primary);
}

compat::optional<codegen::SourceSpan> LiveCodePreviewPanel::GetSpanForNode(compat::string_view nodeUuid) const {
    auto sym = m_impl->cachedResult.sourceMap.FindByNodeUuid(nodeUuid);
    if (!sym.has_value()) {
        return compat::nullopt;
    }
    return sym->spanInDesignerHpp;
}

DesignerEvent<std::pair<PreviewViewMode, std::string>>& LiveCodePreviewPanel::CodeUpdatedSignal() noexcept {
    return m_impl->codeUpdatedSignal;
}

void LiveCodePreviewPanel::OnRender(rendering::DrawingContext& dc) {
    float w = GetWidth();
    float h = GetHeight();
    if (w <= 0.0f) w = 660.0f;
    if (h <= 0.0f) h = 230.0f;

    // 編輯器底色與外框
    dc.DrawRectangle(core::Rect(0.0f, 0.0f, w, h), core::Color(30, 30, 30));
    dc.DrawLine(core::Point(0.0f, 0.0f), core::Point(w, 0.0f), core::Color(51, 51, 51), 1.0f);
    dc.DrawLine(core::Point(0.0f, h - 1.0f), core::Point(w, h - 1.0f), core::Color(51, 51, 51), 1.0f);

    // 左側行號槽
    const float gutterWidth = 44.0f;
    dc.DrawRectangle(core::Rect(0.0f, 0.0f, gutterWidth, h), core::Color(37, 37, 38));
    dc.DrawLine(core::Point(gutterWidth, 0.0f), core::Point(gutterWidth, h), core::Color(51, 51, 51), 1.0f);

    // 切割行字串
    std::istringstream stream(m_impl->activeText);
    std::string line;
    size_t lineIdx = 0;
    const float lineHeight = 18.0f;
    float y = 6.0f;

    while (std::getline(stream, line)) {
        if (y + lineHeight > h) break;

        // 行號 (右對齊)
        std::string lineNoStr = std::to_string(lineIdx + 1);
        float lineNoX = gutterWidth - static_cast<float>(lineNoStr.size() * 8) - 6.0f;
        dc.DrawText(lineNoStr, core::Point(lineNoX, y),
                    text::FontDescriptor("Consolas", 11.0f),
                    core::Color(130, 130, 130));

        // 簡要著色：關鍵字/符號
        core::Color codeColor = core::Color(215, 215, 215);
        if (line.find("#include") != std::string::npos || line.find("#pragma") != std::string::npos) {
            codeColor = core::Color(197, 134, 192);
        } else if (line.find("//") != std::string::npos) {
            codeColor = core::Color(106, 153, 85);
        } else if (line.find("class ") != std::string::npos || line.find("namespace ") != std::string::npos ||
                   line.find("auto ") != std::string::npos || line.find("return ") != std::string::npos) {
            codeColor = core::Color(86, 156, 214);
        }

        dc.DrawText(line, core::Point(gutterWidth + 8.0f, y),
                    text::FontDescriptor("Consolas", 11.5f),
                    codeColor);

        y += lineHeight;
        ++lineIdx;
    }
}

} // namespace preview
} // namespace designer
} // namespace cppui
