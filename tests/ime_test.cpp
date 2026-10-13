/**
 * @file ime_test.cpp
 * @brief 文字輸入法 (IME) 深度整合、組字管線、候選字窗定位與 TextBox 互動單元測試
 */

#include <cassert>
#include <cstdint>
#include <string>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/pal/RawInputEvent.hpp"
#include "ui/input/KeyEventArgs.hpp"
#include "ui/input/RoutedEvent.hpp"
#include "ui/input/InputManager.hpp"
#include "ui/input/FocusManager.hpp"
#include "ui/controls/TextBox.hpp"
#include "ui/layout/StackPanel.hpp"
#include "ui/app/Form.hpp"

using namespace cppui;

/// <summary>
/// 測試 1: 原始輸入事件 (RawInputEvent) 之 IME 欄位與工廠建構驗證
/// </summary>
static void TestRawImeEventFactories() {
    // 組合中事件
    pal::RawInputEvent compEvt = pal::RawInputEvent::MakeImeComposition("注音", 2, false, 12345);
    assert(compEvt.type == pal::RawEventType::ImeComposition);
    assert(compEvt.timestampMs == 12345);
    assert(std::string(compEvt.ime.text) == "注音");
    assert(compEvt.ime.cursorPosition == 2);
    assert(!compEvt.ime.isResult);

    // 定稿確認事件
    pal::RawInputEvent commitEvt = pal::RawInputEvent::MakeImeComposition("確認字元", 0, true, 12346);
    assert(commitEvt.type == pal::RawEventType::ImeComposition);
    assert(commitEvt.timestampMs == 12346);
    assert(std::string(commitEvt.ime.text) == "確認字元");
    assert(commitEvt.ime.cursorPosition == 0);
    assert(commitEvt.ime.isResult);

    // 結束組字事件
    pal::RawInputEvent endEvt = pal::RawInputEvent::MakeImeEndComposition(12347);
    assert(endEvt.type == pal::RawEventType::ImeEndComposition);
    assert(endEvt.timestampMs == 12347);

    compat::println("[PASS] TestRawImeEventFactories passed.");
}

/// <summary>
/// 測試 2: ImeCompositionEventArgs 路由事件參數封裝驗證
/// </summary>
static void TestImeCompositionEventArgs() {
    controls::TextBox tb;
    input::ImeCompositionEventArgs compArgs(
        input::StandardEvents::ImeComposition,
        &tb,
        "測試組字",
        3,
        false
    );

    assert(compArgs.GetCompositionText() == "測試組字");
    assert(compArgs.GetCursorPosition() == 3);
    assert(!compArgs.IsCommitted());
    assert(compArgs.GetSource() == &tb);

    input::ImeCompositionEventArgs commitArgs(
        input::StandardEvents::ImeComposition,
        &tb,
        "定稿文字",
        0,
        true
    );
    assert(commitArgs.GetCompositionText() == "定稿文字");
    assert(commitArgs.GetCursorPosition() == 0);
    assert(commitArgs.IsCommitted());

    compat::println("[PASS] TestImeCompositionEventArgs passed.");
}

/// <summary>
/// 測試 3: InputManager 對焦點控制項之 IME 事件派發驗證
/// </summary>
static void TestInputManagerImeDispatch() {
    auto panel = std::make_shared<layout::StackPanel>();
    auto tb = std::make_shared<controls::TextBox>();
    panel->AddChild(tb);

    // 元件獲焦
    input::FocusManager::Get().SetFocus(tb.get());
    assert(input::FocusManager::Get().GetFocusedElement() == tb.get());

    // 注入組字事件
    pal::RawInputEvent compEvt = pal::RawInputEvent::MakeImeComposition("ㄍ", 1, false);
    input::InputManager::Get().ProcessRawEvent(*panel, compEvt);
    assert(tb->GetCompositionText() == "ㄍ");
    assert(tb->GetCompositionCursor() == 1);
    assert(tb->GetText().empty()); // 本文尚未變更

    // 注入定字確認事件
    pal::RawInputEvent commitEvt = pal::RawInputEvent::MakeImeComposition("個", 0, true);
    input::InputManager::Get().ProcessRawEvent(*panel, commitEvt);
    assert(tb->GetCompositionText().empty()); // 組字暫存已清空
    assert(tb->GetText() == "個");            // 內容已確認

    compat::println("[PASS] TestInputManagerImeDispatch passed.");
}

/// <summary>
/// 測試 4: TextBox 組字流程、連續組字定稿、游標邊界與排版測量驗證
/// </summary>
static void TestTextBoxCompositionLifecycle() {
    controls::TextBox tb;
    tb.SetText("ABC");
    tb.SetCaretIndex(3);

    // 初始游標邊界檢驗
    core::Rect caret1 = tb.GetCaretRect();
    assert(caret1.width > 0.0f);
    assert(caret1.height > 0.0f);

    // 注入組字串 "DE"
    input::ImeCompositionEventArgs compArgs(
        input::StandardEvents::ImeComposition,
        &tb,
        "DE",
        2,
        false
    );
    tb.RaiseEvent(compArgs);
    assert(tb.GetCompositionText() == "DE");
    assert(tb.GetCompositionCursor() == 2);
    assert(tb.GetText() == "ABC"); // 內容未提交

    // 組字中排版測量檢驗 (期望尺寸應包含組字串寬度)
    core::Size initialSize = tb.MeasureOverride(core::Size(1000.0f, 1000.0f));
    assert(initialSize.width > 0.0f);

    // 組字游標邊界檢驗 (游標應位於 ABC + DE 之後)
    core::Rect caret2 = tb.GetCaretRect();
    assert(caret2.x > caret1.x);

    // 確認提交 "DE"
    input::ImeCompositionEventArgs commitArgs(
        input::StandardEvents::ImeComposition,
        &tb,
        "DE",
        0,
        true
    );
    tb.RaiseEvent(commitArgs);
    assert(tb.GetCompositionText().empty());
    assert(tb.GetText() == "ABCDE");
    assert(tb.GetCaretIndex() == 5);

    compat::println("[PASS] TestTextBoxCompositionLifecycle passed.");
}

/// <summary>
/// 測試 5: 邊界情況測試 (唯讀防護、清空重置、失焦清空)
/// </summary>
static void TestTextBoxImeEdgeCases() {
    controls::TextBox tb;
    tb.SetText("ReadOnly");
    tb.SetReadOnly(true);

    // 唯讀模式下組字與定稿應被拒絕
    input::ImeCompositionEventArgs compArgs(
        input::StandardEvents::ImeComposition,
        &tb,
        "嘗試輸入",
        0,
        false
    );
    tb.RaiseEvent(compArgs);
    assert(tb.GetCompositionText().empty());
    assert(tb.GetText() == "ReadOnly");

    // 解除唯讀並測試失焦清空
    tb.SetReadOnly(false);
    input::ImeCompositionEventArgs compArgs2(
        input::StandardEvents::ImeComposition,
        &tb,
        "未確認組字",
        1,
        false
    );
    tb.RaiseEvent(compArgs2);
    assert(tb.GetCompositionText() == "未確認組字");

    // 模擬失焦
    input::FocusEventArgs lostFocusArgs(input::StandardEvents::LostFocus, &tb, &tb, nullptr);
    tb.RaiseEvent(lostFocusArgs);
    assert(tb.GetCompositionText().empty());

    // 測試 Clear() 重置
    input::ImeCompositionEventArgs compArgs3(
        input::StandardEvents::ImeComposition,
        &tb,
        "殘留組字",
        2,
        false
    );
    tb.RaiseEvent(compArgs3);
    assert(tb.GetCompositionText() == "殘留組字");

    tb.Clear();
    assert(tb.GetCompositionText().empty());
    assert(tb.GetText().empty());
    assert(tb.GetCaretIndex() == 0);

    compat::println("[PASS] TestTextBoxImeEdgeCases passed.");
}

int main() {
    compat::println("=== 執行 CPPUI 文字輸入法 (IME) 與組字管線單元測試 ===");
    TestRawImeEventFactories();
    TestImeCompositionEventArgs();
    TestInputManagerImeDispatch();
    TestTextBoxCompositionLifecycle();
    TestTextBoxImeEdgeCases();
    compat::println("=== 所有 IME 單元測試全數通過 (100% PASS) ===");
    return 0;
}
