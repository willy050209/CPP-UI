/**
 * @file CodePreviewWindow.cpp
 * @brief 視覺化表單設計器獨立即時代碼預覽視窗實作
 */

#include "ui/designer/preview/CodePreviewWindow.hpp"
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <unordered_set>
#include "ui/theme/ThemeManager.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/designer/codegen/FormCodegenPipeline.hpp"
#include "ui/pal/IClipboard.hpp"

namespace cppui {
namespace designer {
namespace preview {

namespace {

enum class TokenType {
    Keyword,
    Type,
    String,
    Number,
    Comment,
    Preprocessor,
    Other
};

struct Token {
    std::string text;
    TokenType type;
};

const std::unordered_set<std::string>& GetKeywords() {
    static const std::unordered_set<std::string> s_keywords = {
        "auto", "break", "case", "class", "const", "constexpr", "continue",
        "default", "delete", "do", "else", "enum", "explicit", "export",
        "extern", "false", "for", "friend", "goto", "if", "inline", "mutable",
        "namespace", "new", "noexcept", "nullptr", "operator", "override",
        "private", "protected", "public", "return", "sizeof", "static",
        "struct", "switch", "template", "this", "throw", "true", "try",
        "typedef", "typename", "union", "using", "virtual", "void", "volatile",
        "while", "final"
    };
    return s_keywords;
}

const std::unordered_set<std::string>& GetKnownTypes() {
    static const std::unordered_set<std::string> s_types = {
        "int", "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t",
        "uint32_t", "uint64_t", "size_t", "float", "double", "bool", "char",
        "string", "string_view", "vector", "unique_ptr", "shared_ptr", "optional",
        "expected", "span", "propagate_const", "Button", "CheckBox", "ComboBox",
        "DataGrid", "Label", "ListBox", "PictureBox", "ProgressBar", "RadioButton",
        "Separator", "Slider", "TextBlock", "TextBox", "Timer", "OpenFileDialog",
        "SaveFileDialog", "MessageBox", "Grid", "StackPanel", "Canvas", "Border",
        "ScrollViewer", "Form", "Color", "Rect", "Point", "Thickness", "Size"
    };
    return s_types;
}

std::vector<Token> TokenizeLine(compat::string_view line) {
    std::vector<Token> tokens;
    size_t i = 0;
    size_t n = line.size();

    const auto& keywords = GetKeywords();
    const auto& types = GetKnownTypes();

    while (i < n) {
        // 註解 //
        if (i + 1 < n && line[i] == '/' && line[i + 1] == '/') {
            tokens.push_back({ std::string(line.substr(i)), TokenType::Comment });
            break;
        }

        // 預處理器指令 #include, #pragma 等
        if (line[i] == '#') {
            size_t start = i++;
            while (i < n && (std::isalnum(static_cast<unsigned char>(line[i])) || line[i] == '_')) {
                ++i;
            }
            tokens.push_back({ std::string(line.substr(start, i - start)), TokenType::Preprocessor });
            continue;
        }

        // 字串常面字 "..."
        if (line[i] == '"') {
            size_t start = i++;
            while (i < n && line[i] != '"') {
                if (line[i] == '\\' && i + 1 < n) {
                    i += 2;
                } else {
                    ++i;
                }
            }
            if (i < n) ++i;
            tokens.push_back({ std::string(line.substr(start, i - start)), TokenType::String });
            continue;
        }

        // 識別碼與關鍵字
        if (std::isalpha(static_cast<unsigned char>(line[i])) || line[i] == '_') {
            size_t start = i++;
            while (i < n && (std::isalnum(static_cast<unsigned char>(line[i])) || line[i] == '_')) {
                ++i;
            }
            std::string word(line.substr(start, i - start));
            if (keywords.find(word) != keywords.end()) {
                tokens.push_back({ std::move(word), TokenType::Keyword });
            } else if (types.find(word) != types.end()) {
                tokens.push_back({ std::move(word), TokenType::Type });
            } else {
                tokens.push_back({ std::move(word), TokenType::Other });
            }
            continue;
        }

        // 數值常數
        if (std::isdigit(static_cast<unsigned char>(line[i]))) {
            size_t start = i++;
            while (i < n && (std::isalnum(static_cast<unsigned char>(line[i])) || line[i] == '.' || line[i] == 'f')) {
                ++i;
            }
            tokens.push_back({ std::string(line.substr(start, i - start)), TokenType::Number });
            continue;
        }

        // 其他單字符號或空白
        size_t start = i++;
        while (i < n && !std::isalnum(static_cast<unsigned char>(line[i])) &&
               line[i] != '_' && line[i] != '"' && line[i] != '#' &&
               !(line[i] == '/' && i + 1 < n && line[i + 1] == '/')) {
            ++i;
        }
        tokens.push_back({ std::string(line.substr(start, i - start)), TokenType::Other });
    }

    return tokens;
}

core::Color GetColorForToken(TokenType type) {
    switch (type) {
        case TokenType::Keyword:      return core::Color(86, 156, 214);  // 亮藍
        case TokenType::Type:         return core::Color(78, 201, 176);  // 青綠
        case TokenType::String:       return core::Color(206, 145, 120); // 暖橘
        case TokenType::Number:       return core::Color(181, 206, 168); // 淺綠
        case TokenType::Comment:      return core::Color(106, 153, 85);  // 草綠
        case TokenType::Preprocessor: return core::Color(197, 134, 192); // 洋紫
        case TokenType::Other:        return core::Color(215, 215, 215); // 灰白
    }
    return core::Color(215, 215, 215);
}

} // anonymous namespace

class CodeEditorControl : public controls::Control {
public:
    explicit CodeEditorControl(CodePreviewWindow::Impl* hostImpl)
        : m_host(hostImpl) {}

    void OnRender(rendering::DrawingContext& dc) override;
    void OnPointerPressedCore(input::PointerEventArgs& e) override;

private:
    CodePreviewWindow::Impl* m_host;
};

struct CodePreviewWindow::Impl {
    FormDocumentNode* document = nullptr;
    surface::SelectionModel* selectionModel = nullptr;
    PreviewViewMode viewMode = PreviewViewMode::FluentDsl;

    codegen::FormCodegenPipeline pipeline;
    codegen::CodegenResult cachedResult;
    std::string activeText;
    std::vector<std::string> lines;

    float scrollY = 0.0f;
    std::shared_ptr<CodeEditorControl> editorView;

    reactive::Connection propConn;
    reactive::Connection nodeAddConn;
    reactive::Connection nodeRemConn;
    reactive::Connection nodeMovConn;
    reactive::Connection settingsConn;

    void ConnectSignals() {
        propConn.Disconnect();
        nodeAddConn.Disconnect();
        nodeRemConn.Disconnect();
        nodeMovConn.Disconnect();
        settingsConn.Disconnect();

        if (document) {
            propConn = document->PropertyChangedSignal().Connect([this](const PropertyChangedPayload&) { Refresh(); });
            nodeAddConn = document->NodeAddedSignal().Connect([this](const NodeAddedPayload&) { Refresh(); });
            nodeRemConn = document->NodeRemovedSignal().Connect([this](const NodeRemovedPayload&) { Refresh(); });
            nodeMovConn = document->NodeMovedSignal().Connect([this](const NodeMovedPayload&) { Refresh(); });
            settingsConn = document->SettingsChangedSignal().Connect([this](const FormSettingsChangedPayload&) { Refresh(); });
        }
    }

    void Refresh() {
        if (!document) {
            cachedResult = codegen::CodegenResult{};
            activeText.clear();
            lines.clear();
            if (editorView) editorView->InvalidateVisual();
            return;
        }

        codegen::CodegenOptions opts;
        opts.rootNamespace = document->GetSettings().targetNamespace;
        opts.formClassName = document->GetSettings().className;

        auto res = pipeline.Generate(*document, opts);
        if (res.has_value()) {
            cachedResult = std::move(*res);
            UpdateActiveText();
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

        lines.clear();
        std::istringstream stream(activeText);
        std::string line;
        while (std::getline(stream, line)) {
            lines.push_back(line);
        }
        if (editorView) editorView->InvalidateVisual();
    }
};

void CodeEditorControl::OnRender(rendering::DrawingContext& dc) {
    float w = GetWidth();
    float h = GetHeight();
    if (w <= 0.0f) w = 920.0f;
    if (h <= 0.0f) h = 640.0f;

    // 1. 頂部頁籤欄背景 (TabBar)
    const float tabHeight = 36.0f;
    dc.DrawRectangle(core::Rect(0.0f, 0.0f, w, tabHeight), core::Color(37, 37, 38));
    dc.DrawLine(core::Point(0.0f, tabHeight), core::Point(w, tabHeight), core::Color(51, 51, 51), 1.0f);

    struct TabDef {
        PreviewViewMode mode;
        const char* label;
    };
    const TabDef tabs[] = {
        { PreviewViewMode::FluentDsl,          "1. Fluent DSL (.hpp)" },
        { PreviewViewMode::ClassicDesignerHpp, "2. Designer (.designer.hpp)" },
        { PreviewViewMode::ClassicHeader,      "3. User Header (.hpp)" },
        { PreviewViewMode::ClassicSource,      "4. User Source (.cpp)" },
        { PreviewViewMode::CMakeLists,         "5. CMakeLists.txt" }
    };

    float tabX = 6.0f;
    for (const auto& tab : tabs) {
        bool isActive = (m_host->viewMode == tab.mode);
        float tabW = 150.0f;
        core::Rect tabRect(tabX, 4.0f, tabW, tabHeight - 4.0f);

        core::Color tabBg = isActive ? core::Color(30, 30, 30) : core::Color(45, 45, 48);
        dc.DrawRoundedRectangle(tabRect, 3.0f, tabBg);

        if (isActive) {
            // 標示作用中藍色指示條
            dc.DrawLine(core::Point(tabRect.x, 4.0f), core::Point(tabRect.x + tabRect.width, 4.0f), core::Color(0, 122, 204), 2.0f);
        }

        core::Color textClr = isActive ? core::Color(255, 255, 255) : core::Color(180, 180, 180);
        dc.DrawText(tab.label, core::Point(tabRect.x + 8.0f, tabRect.y + 7.0f),
                    text::FontDescriptor("Segoe UI", 11.0f, isActive ? text::FontWeight::Bold : text::FontWeight::Normal),
                    textClr);
        tabX += tabW + 4.0f;
    }

    // 頂部右側：複製按鈕
    core::Rect copyRect(w - 140.0f, 5.0f, 130.0f, 26.0f);
    dc.DrawRoundedRectangle(copyRect, 4.0f, core::Color(14, 110, 210));
    dc.DrawText("📋 複製代碼 (Copy)", core::Point(copyRect.x + 10.0f, copyRect.y + 5.0f),
                text::FontDescriptor("Segoe UI", 11.5f, text::FontWeight::Bold),
                core::Color(255, 255, 255));

    // 2. 編輯器主視窗底色與左側行號槽 (Gutter)
    const float gutterWidth = 52.0f;
    core::Rect editorRect(0.0f, tabHeight + 1.0f, w, h - tabHeight - 1.0f);
    dc.DrawRectangle(editorRect, core::Color(30, 30, 30));

    core::Rect gutterRect(0.0f, tabHeight + 1.0f, gutterWidth, h - tabHeight - 1.0f);
    dc.DrawRectangle(gutterRect, core::Color(37, 37, 38));
    dc.DrawLine(core::Point(gutterWidth, tabHeight + 1.0f), core::Point(gutterWidth, h), core::Color(51, 51, 51), 1.0f);

    // 3. 代碼行號與高亮逐行渲染
    const float lineHeight = 19.0f;
    float startY = tabHeight + 8.0f - m_host->scrollY;

    for (size_t lineIdx = 0; lineIdx < m_host->lines.size(); ++lineIdx) {
        float lineY = startY + static_cast<float>(lineIdx) * lineHeight;
        if (lineY + lineHeight < tabHeight || lineY > h) {
            continue; // 超出可視區剪裁
        }

        // 行號繪製 (右對齊)
        std::string lineNoStr = std::to_string(lineIdx + 1);
        float lineNoX = gutterWidth - static_cast<float>(lineNoStr.size() * 8) - 10.0f;
        dc.DrawText(lineNoStr, core::Point(lineNoX, lineY),
                    text::FontDescriptor("Consolas", 11.5f),
                    core::Color(133, 133, 133));

        // 語法著色 Tokens 繪製
        auto tokens = TokenizeLine(m_host->lines[lineIdx]);
        float tokenX = gutterWidth + 12.0f;

        for (const auto& tok : tokens) {
            core::Color clr = GetColorForToken(tok.type);
            dc.DrawText(tok.text, core::Point(tokenX, lineY),
                        text::FontDescriptor("Consolas", 12.0f),
                        clr);
            tokenX += static_cast<float>(tok.text.size()) * 7.4f; // 等寬字元間距估算
        }
    }
}

void CodeEditorControl::OnPointerPressedCore(input::PointerEventArgs& e) {
    core::Point pt = e.GetPosition(this);
    float w = GetWidth();
    if (w <= 0.0f) w = 920.0f;

    // 複製按鈕點擊
    core::Rect copyRect(w - 140.0f, 5.0f, 130.0f, 26.0f);
    if (copyRect.Contains(pt)) {
        pal::Clipboard::SetText(m_host->activeText);
        e.SetHandled(true);
        return;
    }

    // 頁籤點擊
    const PreviewViewMode modes[] = {
        PreviewViewMode::FluentDsl,
        PreviewViewMode::ClassicDesignerHpp,
        PreviewViewMode::ClassicHeader,
        PreviewViewMode::ClassicSource,
        PreviewViewMode::CMakeLists
    };

    float tabX = 6.0f;
    const float tabW = 150.0f;
    for (PreviewViewMode m : modes) {
        core::Rect tabRect(tabX, 4.0f, tabW, 32.0f);
        if (tabRect.Contains(pt)) {
            m_host->viewMode = m;
            m_host->UpdateActiveText();
            InvalidateVisual();
            e.SetHandled(true);
            return;
        }
        tabX += tabW + 4.0f;
    }
}

CodePreviewWindow::CodePreviewWindow(
    FormDocumentNode* document,
    surface::SelectionModel* selectionModel
) : m_impl(std::make_unique<Impl>()) {
    m_impl->document = document;
    m_impl->selectionModel = selectionModel;

    SetTitle("⚡ CPPUI Live Code Preview - 即時代碼預覽視窗");
    SetFormSize(940, 680);

    m_impl->editorView = std::make_shared<CodeEditorControl>(m_impl.get());
    m_impl->editorView->SetWidth(940.0f);
    m_impl->editorView->SetHeight(680.0f);
    SetContent(m_impl->editorView);

    m_impl->ConnectSignals();
    m_impl->Refresh();
}

CodePreviewWindow::~CodePreviewWindow() = default;

void CodePreviewWindow::SetDocument(FormDocumentNode* document) {
    m_impl->document = document;
    m_impl->ConnectSignals();
    m_impl->Refresh();
}

void CodePreviewWindow::SetViewMode(PreviewViewMode mode) {
    m_impl->viewMode = mode;
    m_impl->UpdateActiveText();
}

PreviewViewMode CodePreviewWindow::GetViewMode() const noexcept {
    return m_impl->viewMode;
}

void CodePreviewWindow::RefreshCode() {
    m_impl->Refresh();
}

compat::string_view CodePreviewWindow::GetCurrentCode() const noexcept {
    return m_impl->activeText;
}

void CodePreviewWindow::CopyCodeToClipboard() {
    pal::Clipboard::SetText(m_impl->activeText);
}

} // namespace preview
} // namespace designer
} // namespace cppui
