/**
 * @file LiveCodePreviewPanel.cpp
 * @brief CPPUI Designer 即時代碼預覽面板實作
 */

#include "ui/designer/preview/LiveCodePreviewPanel.hpp"
#include "ui/designer/codegen/FormCodegenPipeline.hpp"

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

    reactive::Connection propConn;
    reactive::Connection nodeAddConn;
    reactive::Connection nodeRemConn;
    reactive::Connection nodeMovConn;
    reactive::Connection settingsConn;

    void ConnectDocumentSignals() {
        propConn.Disconnect();
        nodeAddConn.Disconnect();
        nodeRemConn.Disconnect();
        nodeMovConn.Disconnect();
        settingsConn.Disconnect();

        if (document) {
            propConn = document->PropertyChangedSignal().Connect([this](const PropertyChangedPayload&) {
                RefreshCode();
            });
            nodeAddConn = document->NodeAddedSignal().Connect([this](const NodeAddedPayload&) {
                RefreshCode();
            });
            nodeRemConn = document->NodeRemovedSignal().Connect([this](const NodeRemovedPayload&) {
                RefreshCode();
            });
            nodeMovConn = document->NodeMovedSignal().Connect([this](const NodeMovedPayload&) {
                RefreshCode();
            });
            settingsConn = document->SettingsChangedSignal().Connect([this](const FormSettingsChangedPayload&) {
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
    controls::Control::OnRender(dc);
}

} // namespace preview
} // namespace designer
} // namespace cppui
