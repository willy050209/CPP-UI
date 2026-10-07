#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/core/Visual.hpp"
#include "ui/core/UIElement.hpp"
#include "ui/layout/LayoutTypes.hpp"
#include "ui/layout/Panel.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/layout/Border.hpp"
#include "ui/layout/Canvas.hpp"
#include "ui/layout/LayoutManager.hpp"
#include "ui/controls/TextBlock.hpp"
#include "ui/rendering/DisplayList.hpp"
#include "ui/rendering/DrawingContext.hpp"
#include "ui/rendering/DisplayListRenderer.hpp"
#include "../../src/rendering/backend/SoftwareRenderContext.hpp"
#include "ui/pal/IPlatformWindow.hpp"
#include "ui/pal/PlatformWindowFactory.hpp"
#include "ui/input/InputTypes.hpp"
#include "ui/input/RoutedEvent.hpp"
#include "ui/input/RoutedEventArgs.hpp"
#include "ui/input/PointerEventArgs.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/input/FocusEventArgs.hpp"
#include "ui/input/EventRoute.hpp"
#include "ui/input/FocusManager.hpp"
#include "ui/input/InputManager.hpp"

#ifdef _WIN32
#include "../../src/pal/win32/PlatformWindowWin32.hpp"
#endif

using namespace cppui;
using namespace cppui::core;
using namespace cppui::layout;
using namespace cppui::controls;
using namespace cppui::rendering;
using namespace cppui::input;

// =============================================================================
// 自訂互動按鈕元件 (具備 Hover 變光、Pressed 下壓、Focus Ring 霓虹外框與點擊回呼)
// =============================================================================
class ModernButton : public Border {
public:
    ModernButton(const std::string& text, Color normalBg, Color hoverBg, Color pressedBg)
        : m_normalBg(normalBg), m_hoverBg(hoverBg), m_pressedBg(pressedBg) {
        SetBackground(normalBg);
        SetBorderBrush(Color(60, 70, 90, 255));
        SetBorderThickness(Thickness(1.0f));
        SetCornerRadius(6.0f);
        SetPadding(Thickness(16.0f, 10.0f, 16.0f, 10.0f));
        SetFocusable(true);
        SetTabStop(true);

        m_textBlock = std::make_shared<TextBlock>(text);
        m_textBlock->SetFontSize(14.0f);
        m_textBlock->SetFontWeight(text::FontWeight::SemiBold);
        m_textBlock->SetForeground(Color(240, 245, 255, 255));
        m_textBlock->SetHorizontalAlignment(HorizontalAlignment::Center);
        m_textBlock->SetVerticalAlignment(VerticalAlignment::Center);
        m_textBlock->SetHitTestVisible(false); // 穿透至按鈕主體
        SetChild(m_textBlock);

        OnPointerEntered([this](PointerEventArgs&) {
            m_isHovered = true;
            if (!m_isPressed) SetBackground(m_hoverBg);
            InvalidateVisual();
        });

        OnPointerExited([this](PointerEventArgs&) {
            m_isHovered = false;
            m_isPressed = false;
            SetBackground(m_normalBg);
            InvalidateVisual();
        });

        OnPointerPressed([this](PointerEventArgs& e) {
            if (e.IsLeftButton()) {
                m_isPressed = true;
                SetBackground(m_pressedBg);
                InvalidateVisual();
            }
        });

        OnPointerReleased([this](PointerEventArgs& e) {
            if (e.IsLeftButton()) {
                m_isPressed = false;
                SetBackground(m_isHovered ? m_hoverBg : m_normalBg);
                InvalidateVisual();
            }
        });
    }

    void SetText(const std::string& text) {
        if (m_textBlock) m_textBlock->SetText(text);
    }

    void OnRender(DrawingContext& dc) override {
        // 先繪製 Border 原生邊框與背景
        Border::OnRender(dc);

        // 若擁有焦點，外擴繪製醒目現代霓虹 Focus Ring
        if (IsFocused()) {
            Rect ringRect(-2.0f, -2.0f, m_visualBounds.width + 4.0f, m_visualBounds.height + 4.0f);
            dc.DrawRoundedRectangle(ringRect, GetCornerRadius() + 2.0f, Color(70, 160, 255, 220));
        }
    }

private:
    Color m_normalBg;
    Color m_hoverBg;
    Color m_pressedBg;
    bool  m_isHovered = false;
    bool  m_isPressed = false;
    std::shared_ptr<TextBlock> m_textBlock;
};

// =============================================================================
// 自訂即時鍵盤輸入元件 (支援聚焦、字元打字即時回顯、Backspace 退格與光標)
// =============================================================================
class ModernTextInput : public Border {
public:
    ModernTextInput(const std::string& placeholder)
        : m_placeholder(placeholder) {
        SetBackground(Color(18, 20, 26, 255));
        SetBorderBrush(Color(50, 60, 75, 255));
        SetBorderThickness(Thickness(1.5f));
        SetCornerRadius(6.0f);
        SetPadding(Thickness(14.0f, 10.0f, 14.0f, 10.0f));
        SetFocusable(true);
        SetTabStop(true);

        m_textBlock = std::make_shared<TextBlock>(placeholder);
        m_textBlock->SetFontSize(14.0f);
        m_textBlock->SetFontFamily("Consolas");
        m_textBlock->SetForeground(Color(120, 135, 155, 255)); // Placeholder 顏色
        m_textBlock->SetHitTestVisible(false);
        SetChild(m_textBlock);

        OnGotFocus([this](FocusEventArgs&) {
            SetBorderBrush(Color(0, 200, 255, 255));
            UpdateDisplay();
            InvalidateVisual();
        });

        OnLostFocus([this](FocusEventArgs&) {
            SetBorderBrush(Color(50, 60, 75, 255));
            UpdateDisplay();
            InvalidateVisual();
        });

        OnTextInput([this](TextInputEventArgs& e) {
            // 忽略控制字元
            if (e.GetCodepoint() >= 32) {
                m_content.append(e.GetText().data(), e.GetText().size());
                UpdateDisplay();
                InvalidateVisual();
            }
        });

        OnKeyDown([this](KeyEventArgs& e) {
            if (e.GetVirtualKey() == 0x08) { // Backspace
                if (!m_content.empty()) {
                    // 簡單處理 UTF-8 多字節退格
                    while (!m_content.empty() && (static_cast<uint8_t>(m_content.back()) & 0xC0) == 0x80) {
                        m_content.pop_back();
                    }
                    if (!m_content.empty()) {
                        m_content.pop_back();
                    }
                    UpdateDisplay();
                    InvalidateVisual();
                }
            } else if (e.GetVirtualKey() == 0x0D) { // Enter 清空重設
                m_content.clear();
                UpdateDisplay();
                InvalidateVisual();
            }
        });
    }

    void OnRender(DrawingContext& dc) override {
        Border::OnRender(dc);

        if (IsFocused()) {
            Rect ringRect(-2.0f, -2.0f, m_visualBounds.width + 4.0f, m_visualBounds.height + 4.0f);
            dc.DrawRoundedRectangle(ringRect, GetCornerRadius() + 2.0f, Color(0, 210, 255, 200));
        }
    }

    const std::string& GetText() const { return m_content; }

private:
    void UpdateDisplay() {
        if (m_content.empty()) {
            m_textBlock->SetText(IsFocused() ? "│" : m_placeholder);
            m_textBlock->SetForeground(Color(120, 135, 155, 255));
        } else {
            m_textBlock->SetText(m_content + (IsFocused() ? "│" : ""));
            m_textBlock->SetForeground(Color(245, 250, 255, 255));
        }
    }

    std::string m_placeholder;
    std::string m_content;
    std::shared_ptr<TextBlock> m_textBlock;
};

// =============================================================================
// 主程式入口
// =============================================================================
int main() {
    compat::println("=================================================");
    compat::println(" [CPPUI M5] Input & Routed Events Interactive Demo");
    compat::println("=================================================");

    pal::WindowDesc desc;
    desc.title = "CPPUI M5 - 路由事件、游標追蹤與焦點管理系統";
    desc.width = 1180;
    desc.height = 780;
    desc.resizable = true;

    auto window = pal::PlatformWindowFactory::Create(desc);
    if (!window) {
        compat::println(stderr, "Failed to create platform window!");
        return -1;
    }
    window->Show();

    SoftwareRenderTarget target;
    DisplayListRenderer renderer;

    uint32_t currentWidth = desc.width;
    uint32_t currentHeight = desc.height;
    target.Resize(currentWidth, currentHeight);

    // 建立視覺樹根節點
    auto rootBorder = std::make_shared<Border>();
    rootBorder->SetBackground(Color(22, 24, 30, 255));
    rootBorder->SetBorderBrush(Color(45, 50, 62, 255));
    rootBorder->SetBorderThickness(Thickness(1.0f));
    rootBorder->SetPadding(Thickness(20.0f));

    auto mainStack = std::make_shared<StackPanel>();
    mainStack->SetOrientation(Orientation::Vertical);
    mainStack->SetSpacing(16.0f);
    rootBorder->SetChild(mainStack);

    // 標題列
    auto headerPanel = std::make_shared<StackPanel>();
    headerPanel->SetOrientation(Orientation::Horizontal);
    headerPanel->SetSpacing(12.0f);

    auto titleBlock = std::make_shared<TextBlock>("CPPUI M5 - 路由事件與焦點管理系統");
    titleBlock->SetFontSize(22.0f);
    titleBlock->SetFontWeight(text::FontWeight::Bold);
    titleBlock->SetForeground(Color(255, 255, 255, 255));

    auto subTitleBlock = std::make_shared<TextBlock>("(Tunneling 隧道 | Bubbling 冒泡 | Tab 導航 | Focus Ring 光暈 | Handled 阻斷)");
    subTitleBlock->SetFontSize(13.0f);
    subTitleBlock->SetVerticalAlignment(VerticalAlignment::Center);
    subTitleBlock->SetForeground(Color(130, 150, 180, 255));

    headerPanel->AddChild(titleBlock);
    headerPanel->AddChild(subTitleBlock);
    mainStack->AddChild(headerPanel);

    // 雙欄版面容器
    auto contentPanel = std::make_shared<StackPanel>();
    contentPanel->SetOrientation(Orientation::Horizontal);
    contentPanel->SetSpacing(20.0f);
    mainStack->AddChild(contentPanel);

    float colW = (static_cast<float>(currentWidth) - 40.0f - 20.0f) * 0.5f;

    // --- 左欄：互動控制項與展示區 ---
    auto leftColBorder = std::make_shared<Border>();
    leftColBorder->SetWidth(colW);
    leftColBorder->SetBackground(Color(28, 32, 40, 255));
    leftColBorder->SetBorderBrush(Color(45, 50, 65, 255));
    leftColBorder->SetBorderThickness(Thickness(1.0f));
    leftColBorder->SetCornerRadius(8.0f);
    leftColBorder->SetPadding(Thickness(16.0f));

    auto leftColStack = std::make_shared<StackPanel>();
    leftColStack->SetOrientation(Orientation::Vertical);
    leftColStack->SetSpacing(16.0f);
    leftColBorder->SetChild(leftColStack);

    // --- 右欄：即時事件路由日誌檢視面板 ---
    auto rightColBorder = std::make_shared<Border>();
    rightColBorder->SetWidth(colW);
    rightColBorder->SetBackground(Color(18, 20, 25, 255));
    rightColBorder->SetBorderBrush(Color(40, 45, 60, 255));
    rightColBorder->SetBorderThickness(Thickness(1.0f));
    rightColBorder->SetCornerRadius(8.0f);
    rightColBorder->SetPadding(Thickness(16.0f));

    auto rightColStack = std::make_shared<StackPanel>();
    rightColStack->SetOrientation(Orientation::Vertical);
    rightColStack->SetSpacing(10.0f);
    rightColBorder->SetChild(rightColStack);

    auto inspectorTitle = std::make_shared<TextBlock>("📡 即時事件路由傳播日誌 (Event Route Inspector)");
    inspectorTitle->SetFontSize(15.0f);
    inspectorTitle->SetFontWeight(text::FontWeight::SemiBold);
    inspectorTitle->SetForeground(Color(0, 210, 255, 255));
    rightColStack->AddChild(inspectorTitle);

    // 日誌項目容器 (保留最近 8 條)
    std::vector<std::shared_ptr<TextBlock>> logBlocks;
    for (int i = 0; i < 9; ++i) {
        auto tb = std::make_shared<TextBlock>(i == 0 ? "系統就緒，請在左側點擊按鈕或按下 Tab 鍵切換焦點..." : "");
        tb->SetFontSize(12.0f);
        tb->SetFontFamily("Consolas");
        tb->SetForeground(i == 0 ? Color(140, 155, 175, 255) : Color(180, 195, 215, 255));
        logBlocks.push_back(tb);
        rightColStack->AddChild(tb);
    }

    auto appendLog = [&](const std::string& line) {
        for (size_t i = logBlocks.size() - 1; i > 0; --i) {
            logBlocks[i]->SetText(logBlocks[i - 1]->GetText());
        }
        logBlocks[0]->SetText(line);
        window->RequestRedraw();
    };

    // --- 左欄內容 1：互動式按鈕組 ---
    auto card1 = std::make_shared<Border>();
    card1->SetBackground(Color(36, 40, 52, 255));
    card1->SetCornerRadius(6.0f);
    card1->SetPadding(Thickness(14.0f));

    auto card1Stack = std::make_shared<StackPanel>();
    card1Stack->SetOrientation(Orientation::Vertical);
    card1Stack->SetSpacing(10.0f);
    card1->SetChild(card1Stack);

    auto c1Title = std::make_shared<TextBlock>("1. 互動按鈕群組 (Hover 狀態、Pressed 反饋、冒泡點擊)");
    c1Title->SetFontSize(14.0f);
    c1Title->SetFontWeight(text::FontWeight::SemiBold);
    c1Title->SetForeground(Color(255, 255, 255, 255));
    card1Stack->AddChild(c1Title);

    auto btnRow = std::make_shared<StackPanel>();
    btnRow->SetOrientation(Orientation::Horizontal);
    btnRow->SetSpacing(12.0f);
    card1Stack->AddChild(btnRow);

    int likeCount = 0;
    int rocketCount = 0;

    auto btnLike = std::make_shared<ModernButton>("👍 讚數: 0", Color(45, 75, 130, 255), Color(60, 100, 175, 255), Color(35, 60, 105, 255));
    btnLike->SetTabIndex(1);

    auto btnRocket = std::make_shared<ModernButton>("🚀 發射: 0", Color(130, 60, 45, 255), Color(175, 80, 60, 255), Color(105, 45, 35, 255));
    btnRocket->SetTabIndex(2);

    auto btnReset = std::make_shared<ModernButton>("⚡ 重設", Color(50, 55, 70, 255), Color(70, 75, 95, 255), Color(40, 45, 55, 255));
    btnReset->SetTabIndex(3);

    btnLike->OnPointerReleased([&](PointerEventArgs&) {
        likeCount++;
        btnLike->SetText("👍 讚數: " + std::to_string(likeCount));
        appendLog("[Click Bubble] Target: btnLike | 讚數累計: " + std::to_string(likeCount));
    });

    btnRocket->OnPointerReleased([&](PointerEventArgs&) {
        rocketCount++;
        btnRocket->SetText("🚀 發射: " + std::to_string(rocketCount));
        appendLog("[Click Bubble] Target: btnRocket | 火箭發射: " + std::to_string(rocketCount));
    });

    btnReset->OnPointerReleased([&](PointerEventArgs&) {
        likeCount = 0;
        rocketCount = 0;
        btnLike->SetText("👍 讚數: 0");
        btnRocket->SetText("🚀 發射: 0");
        appendLog("[Click Bubble] Target: btnReset | 計數器已全數歸零");
    });

    btnRow->AddChild(btnLike);
    btnRow->AddChild(btnRocket);
    btnRow->AddChild(btnReset);
    leftColStack->AddChild(card1);

    // --- 左欄內容 2：鍵盤焦點導航與 Tab 順序說明 ---
    auto card2 = std::make_shared<Border>();
    card2->SetBackground(Color(36, 40, 52, 255));
    card2->SetCornerRadius(6.0f);
    card2->SetPadding(Thickness(14.0f));

    auto card2Stack = std::make_shared<StackPanel>();
    card2Stack->SetOrientation(Orientation::Vertical);
    card2Stack->SetSpacing(10.0f);
    card2->SetChild(card2Stack);

    auto c2Title = std::make_shared<TextBlock>("2. Tab 鍵盤導航 (按 [Tab] / [Shift+Tab] 切換焦點)");
    c2Title->SetFontSize(14.0f);
    c2Title->SetFontWeight(text::FontWeight::SemiBold);
    c2Title->SetForeground(Color(255, 255, 255, 255));
    card2Stack->AddChild(c2Title);

    auto tabTip = std::make_shared<TextBlock>("焦點將依 TabIndex: 1(讚) -> 2(火箭) -> 3(重設) -> 4(輸入框) -> 5(阻斷開關) 循環導航！");
    tabTip->SetFontSize(12.0f);
    tabTip->SetForeground(Color(160, 180, 205, 255));
    card2Stack->AddChild(tabTip);
    leftColStack->AddChild(card2);

    // --- 左欄內容 3：即時文字打字輸入盒 ---
    auto card3 = std::make_shared<Border>();
    card3->SetBackground(Color(36, 40, 52, 255));
    card3->SetCornerRadius(6.0f);
    card3->SetPadding(Thickness(14.0f));

    auto card3Stack = std::make_shared<StackPanel>();
    card3Stack->SetOrientation(Orientation::Vertical);
    card3Stack->SetSpacing(10.0f);
    card3->SetChild(card3Stack);

    auto c3Title = std::make_shared<TextBlock>("3. 即時鍵盤文字輸入 (TextInput 路由事件)");
    c3Title->SetFontSize(14.0f);
    c3Title->SetFontWeight(text::FontWeight::SemiBold);
    c3Title->SetForeground(Color(255, 255, 255, 255));
    card3Stack->AddChild(c3Title);

    auto inputControl = std::make_shared<ModernTextInput>("點擊此處獲焦，支援中英文打字與 Backspace 退格...");
    inputControl->SetTabIndex(4);
    card3Stack->AddChild(inputControl);
    leftColStack->AddChild(card3);

    // --- 左欄內容 4：事件短路阻斷測試開關 ---
    auto card4 = std::make_shared<Border>();
    card4->SetBackground(Color(36, 40, 52, 255));
    card4->SetCornerRadius(6.0f);
    card4->SetPadding(Thickness(14.0f));

    auto card4Stack = std::make_shared<StackPanel>();
    card4Stack->SetOrientation(Orientation::Vertical);
    card4Stack->SetSpacing(10.0f);
    card4->SetChild(card4Stack);

    auto c4Title = std::make_shared<TextBlock>("4. 事件短路傳播測試 (Handled Short-Circuiting)");
    c4Title->SetFontSize(14.0f);
    c4Title->SetFontWeight(text::FontWeight::SemiBold);
    c4Title->SetForeground(Color(255, 255, 255, 255));
    card4Stack->AddChild(c4Title);

    bool interceptEnabled = false;
    auto btnToggleIntercept = std::make_shared<ModernButton>("狀態: [正常冒泡傳播] (點擊切換為 Preview 攔截)", Color(40, 50, 65, 255), Color(60, 75, 95, 255), Color(30, 40, 55, 255));
    btnToggleIntercept->SetTabIndex(5);

    btnToggleIntercept->OnPointerReleased([&](PointerEventArgs&) {
        interceptEnabled = !interceptEnabled;
        if (interceptEnabled) {
            btnToggleIntercept->SetText("狀態: [已啟用 Preview 攔截！] (讚/火箭按鈕事件將被阻斷)");
            appendLog("[Intercept] 已開啟 PreviewPointerPressed 攔截阻斷模式！");
        } else {
            btnToggleIntercept->SetText("狀態: [正常冒泡傳播] (點擊切換為 Preview 攔截)");
            appendLog("[Intercept] 已關閉攔截，恢復正常雙向事件傳播！");
        }
    });

    // 於 Card1 註冊 PreviewPointerPressed 攔截器
    card1->AddHandler(StandardEvents::PreviewPointerPressed, [&](RoutedEventArgs& e) {
        if (interceptEnabled) {
            e.SetHandled(true);
            appendLog("[Preview Intercepted!] Card1 攔截了 PreviewPointerPressed 並標記 Handled=true！");
        }
    });

    card4Stack->AddChild(btnToggleIntercept);
    leftColStack->AddChild(card4);

    // 狀態列 (Footer)
    auto footerBorder = std::make_shared<Border>();
    footerBorder->SetBackground(Color(18, 20, 24, 255));
    footerBorder->SetBorderBrush(Color(36, 40, 50, 255));
    footerBorder->SetBorderThickness(Thickness(1.0f));
    footerBorder->SetCornerRadius(4.0f);
    footerBorder->SetPadding(Thickness(14.0f, 8.0f, 14.0f, 8.0f));

    auto statusText = std::make_shared<TextBlock>("【輸入管線即時狀態】 焦點元件: None | 游標狀態: Active | 路由中樞: Running");
    statusText->SetFontSize(12.0f);
    statusText->SetFontFamily("Consolas");
    statusText->SetForeground(Color(140, 160, 180, 255));
    footerBorder->SetChild(statusText);
    mainStack->AddChild(footerBorder);

    contentPanel->AddChild(leftColBorder);
    contentPanel->AddChild(rightColBorder);

    // 視窗排版與繪製回呼
    auto updateLayoutAndRender = [&]() {
        Size availSize(static_cast<float>(currentWidth), static_cast<float>(currentHeight));
        LayoutManager::Get().UpdateLayout(*rootBorder, availSize);

        target.Clear(Color(22, 24, 30, 255));
        DisplayList dl;
        DrawingContext dc(dl);
        rootBorder->Render(dc);
        renderer.Render(dl, &target);

        // 刷新狀態列字串
        UIElement* foc = FocusManager::Get().GetFocusedElement();
        std::string focName = "None";
        if (foc == btnLike.get()) focName = "btnLike (讚數)";
        else if (foc == btnRocket.get()) focName = "btnRocket (火箭)";
        else if (foc == btnReset.get()) focName = "btnReset (重設)";
        else if (foc == inputControl.get()) focName = "inputControl (輸入框)";
        else if (foc == btnToggleIntercept.get()) focName = "btnToggleIntercept (阻斷開關)";

        statusText->SetText("【輸入管線即時狀態】 焦點元件: " + focName + " | 視窗解析度: " + std::to_string(currentWidth) + "x" + std::to_string(currentHeight));
    };

    window->SetPaintCallback([&]() {
        updateLayoutAndRender();

        #ifdef _WIN32
        HWND hwnd = static_cast<HWND>(window->GetNativeHandle());
        HDC hdc = ::GetDC(hwnd);
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = static_cast<LONG>(currentWidth);
        bmi.bmiHeader.biHeight = -static_cast<LONG>(currentHeight);
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        ::StretchDIBits(
            hdc, 0, 0, currentWidth, currentHeight,
            0, 0, currentWidth, currentHeight,
            target.GetPixelBuffer(), &bmi, DIB_RGB_COLORS, SRCCOPY
        );
        ::ReleaseDC(hwnd, hdc);
        #endif
    });

    // 平台原生事件接駁
    window->SetEventCallback([&](const pal::RawInputEvent& e) {
        if (e.type == pal::RawEventType::WindowResize) {
            currentWidth = e.window.width;
            currentHeight = e.window.height;
            target.Resize(currentWidth, currentHeight);
            float newColW = (std::max)(340.0f, (static_cast<float>(currentWidth) - 40.0f - 20.0f) * 0.5f);
            leftColBorder->SetWidth(newColW);
            rightColBorder->SetWidth(newColW);
            rightColBorder->SetHeight((std::max)(420.0f, static_cast<float>(currentHeight) - 220.0f));
            window->RequestRedraw();
            return;
        }

        InputManager::Get().ProcessRawEvent(*rootBorder, e);
        window->RequestRedraw();
    });

    updateLayoutAndRender();
    window->RequestRedraw();

    compat::println("M5 Input Demo 視窗已成功啟動！請在視窗中進行滑鼠與鍵盤互動...");

    // 主訊息迴圈
    while (!window->IsClosed()) {
        window->WaitEvents(16);
    }

    compat::println("M5 Input Demo 結束執行。");
    return 0;
}
