/**
 * @file CodePreviewWindow.cpp
 * @brief 視覺化表單設計器獨立即時代碼預覽視窗實作
 */

#include "ui/designer/preview/CodePreviewWindow.hpp"
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <unordered_set>
#include "ui/theme/ThemeManager.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/designer/codegen/FormCodegenPipeline.hpp"
#include "ui/pal/IClipboard.hpp"
#include "ui/text/TextLayoutEngine.hpp"

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
        "expected", "span", "propagate_const", "function", "pair",
        "Button", "CheckBox", "ComboBox", "DataGrid", "Label", "ListBox",
        "PictureBox", "ProgressBar", "RadioButton", "Separator", "Slider",
        "TextBlock", "TextBox", "Timer", "TreeView", "OpenFileDialog",
        "SaveFileDialog", "MessageBox", "Grid", "StackPanel", "Canvas", "Border",
        "ScrollViewer", "Form", "Color", "Rect", "Point", "Thickness", "Size",
        "SizeToContent", "WindowState", "DialogResult", "NodeType", "WidgetNode",
        "FormDocumentNode", "CommandHistoryManager", "DesignSurface", "SelectionModel",
        "LayoutManager", "ThemeManager", "Theme", "DrawingContext", "DisplayList",
        "PointerEventArgs", "KeyEventArgs", "PropertyDescriptor", "PropertyValue"
    };
    return s_types;
}

std::vector<Token> TokenizeLine(compat::string_view line, bool& inBlockComment) {
    std::vector<Token> tokens;
    size_t i = 0;
    const size_t n = line.size();

    const auto& keywords = GetKeywords();
    const auto& types = GetKnownTypes();

    bool expectingClassName = false;

    // 若上一行結尾仍在區塊註解內
    if (inBlockComment) {
        size_t endComment = line.find("*/");
        if (endComment == compat::string_view::npos) {
            tokens.push_back({ std::string(line), TokenType::Comment });
            return tokens;
        } else {
            tokens.push_back({ std::string(line.substr(0, endComment + 2)), TokenType::Comment });
            i = endComment + 2;
            inBlockComment = false;
        }
    }

    while (i < n) {
        // 單行註解 //
        if (i + 1 < n && line[i] == '/' && line[i + 1] == '/') {
            tokens.push_back({ std::string(line.substr(i)), TokenType::Comment });
            break;
        }

        // 多行註解起始 /* ... */
        if (i + 1 < n && line[i] == '/' && line[i + 1] == '*') {
            size_t endComment = line.find("*/", i + 2);
            if (endComment == compat::string_view::npos) {
                tokens.push_back({ std::string(line.substr(i)), TokenType::Comment });
                inBlockComment = true;
                break;
            } else {
                tokens.push_back({ std::string(line.substr(i, endComment + 2 - i)), TokenType::Comment });
                i = endComment + 2;
                continue;
            }
        }

        // 預處理器指令 #include, #pragma, #define 等
        if (line[i] == '#') {
            size_t start = i++;
            while (i < n && (std::isalnum(static_cast<unsigned char>(line[i])) || line[i] == '_')) {
                ++i;
            }
            std::string directive(line.substr(start, i - start));
            tokens.push_back({ directive, TokenType::Preprocessor });

            // 若為 #include，接續處理標頭檔路徑 <...> 或 "..."
            if (directive == "#include") {
                // 吃掉空白
                while (i < n && std::isspace(static_cast<unsigned char>(line[i]))) {
                    tokens.push_back({ std::string(1, line[i]), TokenType::Other });
                    ++i;
                }
                if (i < n && (line[i] == '<' || line[i] == '"')) {
                    char closing = (line[i] == '<') ? '>' : '"';
                    size_t headerStart = i++;
                    while (i < n && line[i] != closing) {
                        ++i;
                    }
                    if (i < n) ++i;
                    tokens.push_back({ std::string(line.substr(headerStart, i - headerStart)),
                                       closing == '>' ? TokenType::Preprocessor : TokenType::String });
                }
            }
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

        // 識別碼、關鍵字或型別
        if (std::isalpha(static_cast<unsigned char>(line[i])) || line[i] == '_') {
            size_t start = i++;
            while (i < n && (std::isalnum(static_cast<unsigned char>(line[i])) || line[i] == '_')) {
                ++i;
            }
            std::string word(line.substr(start, i - start));

            if (expectingClassName) {
                // class 或 struct 後方的名稱識別碼高亮為 Type
                tokens.push_back({ std::move(word), TokenType::Type });
                expectingClassName = false;
            } else if (word == "class" || word == "struct") {
                tokens.push_back({ word, TokenType::Keyword });
                expectingClassName = true;
            } else if (keywords.find(word) != keywords.end()) {
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

        // 空白字元
        if (std::isspace(static_cast<unsigned char>(line[i]))) {
            size_t start = i++;
            while (i < n && std::isspace(static_cast<unsigned char>(line[i]))) {
                ++i;
            }
            tokens.push_back({ std::string(line.substr(start, i - start)), TokenType::Other });
            continue;
        }

        // 其他單字符號
        tokens.push_back({ std::string(1, line[i]), TokenType::Other });
        ++i;
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

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
float GetConsolasCharWidth(float fontSize) {
    static float s_cachedSize = 0.0f;
    static float s_cachedWidth = 7.4f;
    if (std::abs(s_cachedSize - fontSize) < 0.01f) {
        return s_cachedWidth;
    }
    HDC hdc = GetDC(nullptr);
    if (hdc) {
        int32_t fontH = static_cast<int32_t>(fontSize);
        HFONT hFont = CreateFontW(
            fontH, 0, 0, 0,
            FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Consolas"
        );
        HGDIOBJ old = SelectObject(hdc, hFont);
        SIZE sz = {};
        std::wstring testStr(100, L'M');
        if (GetTextExtentPoint32W(hdc, testStr.data(), static_cast<int>(testStr.size()), &sz) && sz.cx > 0) {
            s_cachedWidth = static_cast<float>(sz.cx) / 100.0f;
            s_cachedSize = fontSize;
        }
        SelectObject(hdc, old);
        DeleteObject(hFont);
        ReleaseDC(nullptr, hdc);
        return s_cachedWidth;
    }
    return fontSize * 0.55f;
}
#else
float GetConsolasCharWidth(float fontSize) {
    return fontSize * 0.55f;
}
#endif

} // anonymous namespace

class CodeEditorControl : public controls::Control {
public:
    explicit CodeEditorControl(CodePreviewWindow::Impl* hostImpl)
        : m_host(hostImpl) {}

    void OnRender(rendering::DrawingContext& dc) override;
    void OnPointerPressedCore(input::PointerEventArgs& e) override;
    void OnPointerMovedCore(input::PointerEventArgs& e) override;
    void OnPointerReleasedCore(input::PointerEventArgs& e) override;
    void OnPointerWheelCore(input::PointerEventArgs& e) override;

private:
    CodePreviewWindow::Impl* m_host;
    bool m_isDraggingScrollbar = false;
    float m_dragStartY = 0.0f;
    float m_dragStartScrollY = 0.0f;
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
    float codeFontSize = 13.5f; // 預設放大至 13.5pt，增強閱讀體驗
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
        scrollY = 0.0f; // 切換分頁時重設捲動位置至頂部
        if (editorView) editorView->InvalidateVisual();
    }
};

void CodeEditorControl::OnRender(rendering::DrawingContext& dc) {
    // 依據目前客戶區大小動態展開 (Stretch)，隨視窗大小縮放適應
    float w = GetActualSize().width > 0.0f ? GetActualSize().width : (GetWidth() > 0.0f ? GetWidth() : 940.0f);
    float h = GetActualSize().height > 0.0f ? GetActualSize().height : (GetHeight() > 0.0f ? GetHeight() : 680.0f);

    // 1. 頂部頁籤欄背景 (TabBar)
    const float tabHeight = 38.0f;
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
        float tabW = 155.0f;
        core::Rect tabRect(tabX, 4.0f, tabW, tabHeight - 4.0f);

        core::Color tabBg = isActive ? core::Color(30, 30, 30) : core::Color(45, 45, 48);
        dc.DrawRoundedRectangle(tabRect, 3.0f, tabBg);

        if (isActive) {
            // 標示作用中藍色指示條
            dc.DrawLine(core::Point(tabRect.x, 4.0f), core::Point(tabRect.x + tabRect.width, 4.0f), core::Color(0, 122, 204), 2.5f);
        }

        core::Color textClr = isActive ? core::Color(255, 255, 255) : core::Color(180, 180, 180);
        dc.DrawText(tab.label, core::Point(tabRect.x + 8.0f, tabRect.y + 8.0f),
                    text::FontDescriptor("Segoe UI", 12.0f, isActive ? text::FontWeight::Bold : text::FontWeight::Normal),
                    textClr);
        tabX += tabW + 4.0f;
    }

    // 頂部右側：字型縮放提示與複製按鈕
    std::string zoomLabel = std::to_string(static_cast<int32_t>(std::round(m_host->codeFontSize / 13.5f * 100.0f))) + "%";
    dc.DrawText(zoomLabel, core::Point(w - 210.0f, 10.0f),
                text::FontDescriptor("Segoe UI", 11.5f), core::Color(160, 160, 160));

    core::Rect copyRect(w - 145.0f, 5.0f, 135.0f, 28.0f);
    dc.DrawRoundedRectangle(copyRect, 4.0f, core::Color(14, 110, 210));
    dc.DrawText("📋 複製代碼 (Copy)", core::Point(copyRect.x + 10.0f, copyRect.y + 5.0f),
                text::FontDescriptor("Segoe UI", 12.0f, text::FontWeight::Bold),
                core::Color(255, 255, 255));

    // 2. 編輯器主視窗底色與左側行號槽 (Gutter)
    text::FontDescriptor codeFont("Consolas", m_host->codeFontSize);
    // 使用精確 GDI 100 字元取樣量測，取得 100% 完全契合 GDI 渲染的等寬步進寬度
    float charWidth = GetConsolasCharWidth(m_host->codeFontSize);
    const float lineHeight = std::ceil(m_host->codeFontSize * 1.55f);

    size_t lineCount = m_host->lines.size();
    size_t digits = std::to_string(lineCount > 0 ? lineCount : 1).size();
    float gutterWidth = (std::max)(50.0f, static_cast<float>(digits) * charWidth + 24.0f);

    core::Rect editorRect(0.0f, tabHeight + 1.0f, w, h - tabHeight - 1.0f);
    dc.DrawRectangle(editorRect, core::Color(30, 30, 30));

    core::Rect gutterRect(0.0f, tabHeight + 1.0f, gutterWidth, h - tabHeight - 1.0f);
    dc.DrawRectangle(gutterRect, core::Color(37, 37, 38));
    dc.DrawLine(core::Point(gutterWidth, tabHeight + 1.0f), core::Point(gutterWidth, h), core::Color(51, 51, 51), 1.0f);

    // 3. 代碼行號與高亮逐行渲染 (具備多行註解跨行狀態機)
    float startY = tabHeight + 6.0f - m_host->scrollY;
    bool inBlockComment = false;

    for (size_t lineIdx = 0; lineIdx < m_host->lines.size(); ++lineIdx) {
        float lineY = startY + static_cast<float>(lineIdx) * lineHeight;

        // 剖析該行 Tokens (更新 inBlockComment 狀態)
        auto tokens = TokenizeLine(m_host->lines[lineIdx], inBlockComment);

        if (lineY + lineHeight < tabHeight || lineY > h) {
            continue; // 超出可視區剪裁以優化效能
        }

        // 行號繪製 (靠右對齊)
        std::string lineNoStr = std::to_string(lineIdx + 1);
        float lineNoX = gutterWidth - static_cast<float>(lineNoStr.size()) * (charWidth * 0.9f) - 10.0f;
        dc.DrawText(lineNoStr, core::Point(lineNoX, lineY + 1.0f),
                    text::FontDescriptor("Consolas", m_host->codeFontSize * 0.9f),
                    core::Color(133, 133, 133));

        // 語法著色 Tokens 繪製 (依據 charWidth 精確等寬步進)
        float tokenX = gutterWidth + 12.0f;
        for (const auto& tok : tokens) {
            core::Color clr = GetColorForToken(tok.type);
            dc.DrawText(tok.text, core::Point(tokenX, lineY), codeFont, clr);
            tokenX += static_cast<float>(tok.text.size()) * charWidth;
        }
    }

    // 4. 右側垂直捲軸渲染 (Scrollbar)
    const float scrollbarWidth = 12.0f;
    const float scrollTrackH = h - tabHeight - 1.0f;
    const float totalContentH = static_cast<float>(m_host->lines.size()) * lineHeight + 40.0f;

    if (totalContentH > scrollTrackH && scrollTrackH > 30.0f) {
        core::Rect trackRect(w - scrollbarWidth, tabHeight + 1.0f, scrollbarWidth, scrollTrackH);
        dc.DrawRectangle(trackRect, core::Color(35, 35, 36));

        float thumbH = (std::max)(28.0f, scrollTrackH * (scrollTrackH / totalContentH));
        float maxScroll = totalContentH - scrollTrackH;
        float scrollRatio = m_host->scrollY / maxScroll;
        scrollRatio = compat::clamp(scrollRatio, 0.0f, 1.0f);
        float thumbY = tabHeight + 1.0f + scrollRatio * (scrollTrackH - thumbH);

        core::Rect thumbRect(w - scrollbarWidth + 2.0f, thumbY, scrollbarWidth - 4.0f, thumbH);
        dc.DrawRoundedRectangle(thumbRect, 3.0f, core::Color(90, 90, 95));
    }
}

void CodeEditorControl::OnPointerPressedCore(input::PointerEventArgs& e) {
    core::Point pt = e.GetPosition(this);
    float w = GetActualSize().width > 0.0f ? GetActualSize().width : (GetWidth() > 0.0f ? GetWidth() : 940.0f);
    float h = GetActualSize().height > 0.0f ? GetActualSize().height : (GetHeight() > 0.0f ? GetHeight() : 680.0f);
    const float tabHeight = 38.0f;

    // 1. 複製按鈕點擊
    core::Rect copyRect(w - 145.0f, 5.0f, 135.0f, 28.0f);
    if (copyRect.Contains(pt)) {
        pal::Clipboard::SetText(m_host->activeText);
        e.SetHandled(true);
        return;
    }

    // 2. 頁籤點擊
    const PreviewViewMode modes[] = {
        PreviewViewMode::FluentDsl,
        PreviewViewMode::ClassicDesignerHpp,
        PreviewViewMode::ClassicHeader,
        PreviewViewMode::ClassicSource,
        PreviewViewMode::CMakeLists
    };

    float tabX = 6.0f;
    const float tabW = 155.0f;
    for (PreviewViewMode m : modes) {
        core::Rect tabRect(tabX, 4.0f, tabW, tabHeight - 4.0f);
        if (tabRect.Contains(pt)) {
            m_host->viewMode = m;
            m_host->UpdateActiveText();
            InvalidateVisual();
            e.SetHandled(true);
            return;
        }
        tabX += tabW + 4.0f;
    }

    // 3. 點擊捲軸軌道或滑塊
    const float scrollbarWidth = 14.0f;
    if (pt.x >= w - scrollbarWidth && pt.y >= tabHeight && pt.y <= h) {
        m_isDraggingScrollbar = true;
        m_dragStartY = pt.y;
        m_dragStartScrollY = m_host->scrollY;
        e.SetHandled(true);
        return;
    }
}

void CodeEditorControl::OnPointerMovedCore(input::PointerEventArgs& e) {
    if (m_isDraggingScrollbar) {
        core::Point pt = e.GetPosition(this);
        float h = GetActualSize().height > 0.0f ? GetActualSize().height : (GetHeight() > 0.0f ? GetHeight() : 680.0f);
        const float tabHeight = 38.0f;
        const float lineHeight = std::ceil(m_host->codeFontSize * 1.55f);
        const float scrollTrackH = h - tabHeight - 1.0f;
        const float totalContentH = static_cast<float>(m_host->lines.size()) * lineHeight + 40.0f;

        if (totalContentH > scrollTrackH && scrollTrackH > 30.0f) {
            float maxScroll = totalContentH - scrollTrackH;
            float dy = pt.y - m_dragStartY;
            float scrollDelta = (dy / scrollTrackH) * totalContentH;
            m_host->scrollY = compat::clamp(m_dragStartScrollY + scrollDelta, 0.0f, maxScroll);
            InvalidateVisual();
        }
        e.SetHandled(true);
    }
}

void CodeEditorControl::OnPointerReleasedCore(input::PointerEventArgs& e) {
    if (m_isDraggingScrollbar) {
        m_isDraggingScrollbar = false;
        e.SetHandled(true);
    }
}

void CodeEditorControl::OnPointerWheelCore(input::PointerEventArgs& e) {
    int32_t delta = e.GetWheelDeltaY();
    if (delta == 0) return;

    // Ctrl + 滾輪：字型大小縮放 (9.0f ~ 32.0f)
    if (e.HasControl()) {
        if (delta > 0) {
            m_host->codeFontSize = compat::clamp(m_host->codeFontSize + 1.0f, 9.0f, 32.0f);
        } else {
            m_host->codeFontSize = compat::clamp(m_host->codeFontSize - 1.0f, 9.0f, 32.0f);
        }
        InvalidateVisual();
        e.SetHandled(true);
        return;
    }

    // 一般滾輪：垂直流暢捲動
    float h = GetActualSize().height > 0.0f ? GetActualSize().height : (GetHeight() > 0.0f ? GetHeight() : 680.0f);
    const float tabHeight = 38.0f;
    const float lineHeight = std::ceil(m_host->codeFontSize * 1.55f);
    const float scrollTrackH = h - tabHeight - 1.0f;
    const float totalContentH = static_cast<float>(m_host->lines.size()) * lineHeight + 40.0f;
    float maxScroll = (std::max)(0.0f, totalContentH - scrollTrackH);

    float scrollStep = lineHeight * 3.0f;
    if (delta > 0) {
        m_host->scrollY = (std::max)(0.0f, m_host->scrollY - scrollStep);
    } else {
        m_host->scrollY = (std::min)(maxScroll, m_host->scrollY + scrollStep);
    }

    InvalidateVisual();
    e.SetHandled(true);
}

CodePreviewWindow::CodePreviewWindow(
    FormDocumentNode* document,
    surface::SelectionModel* selectionModel
) : m_impl(std::make_unique<Impl>()) {
    m_impl->document = document;
    m_impl->selectionModel = selectionModel;

    SetTitle("⚡ CPPUI Live Code Preview - 即時代碼預覽視窗");
    SetFormSize(960, 700);

    m_impl->editorView = std::make_shared<CodeEditorControl>(m_impl.get());
    // 設置拉伸對齊，隨視窗大小縮放自適應
    m_impl->editorView->SetHorizontalAlignment(layout::HorizontalAlignment::Stretch);
    m_impl->editorView->SetVerticalAlignment(layout::VerticalAlignment::Stretch);
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
