/**
 * @file key_test.cpp
 * @brief 鍵盤按鍵碼強型別列舉 (Key) 與分類工具函式單元測試
 */

#include <cassert>
#include <iostream>
#include "compat/compat.hpp"
#include "ui/input/Key.hpp"
#include "ui/input/KeyEventArgs.hpp"

using namespace cppui;
using namespace cppui::input;

static void TestKeyEnumValues() {
    // 導航與基本控制
    assert(static_cast<uint32_t>(Key::None) == 0);
    assert(static_cast<uint32_t>(Key::Backspace) == 0x08);
    assert(Key::Back == Key::Backspace);
    assert(static_cast<uint32_t>(Key::Tab) == 0x09);
    assert(static_cast<uint32_t>(Key::Return) == 0x0D);
    assert(Key::Enter == Key::Return);
    assert(static_cast<uint32_t>(Key::Shift) == 0x10);
    assert(static_cast<uint32_t>(Key::Control) == 0x11);
    assert(Key::Ctrl == Key::Control);
    assert(static_cast<uint32_t>(Key::Menu) == 0x12);
    assert(Key::Alt == Key::Menu);
    assert(static_cast<uint32_t>(Key::Escape) == 0x1B);
    assert(Key::Esc == Key::Escape);
    assert(static_cast<uint32_t>(Key::Space) == 0x20);

    // 箭頭與翻頁導航
    assert(static_cast<uint32_t>(Key::PageUp) == 0x21);
    assert(static_cast<uint32_t>(Key::PageDown) == 0x22);
    assert(static_cast<uint32_t>(Key::End) == 0x23);
    assert(static_cast<uint32_t>(Key::Home) == 0x24);
    assert(static_cast<uint32_t>(Key::Left) == 0x25);
    assert(static_cast<uint32_t>(Key::Up) == 0x26);
    assert(static_cast<uint32_t>(Key::Right) == 0x27);
    assert(static_cast<uint32_t>(Key::Down) == 0x28);
    assert(static_cast<uint32_t>(Key::Delete) == 0x2E);
    assert(Key::Del == Key::Delete);

    // 數字鍵 (0 - 9)
    assert(static_cast<uint32_t>(Key::D0) == 0x30);
    assert(Key::Num0 == Key::D0);
    assert(static_cast<uint32_t>(Key::D9) == 0x39);
    assert(Key::Num9 == Key::D9);

    // 字母鍵 (A - Z)
    assert(static_cast<uint32_t>(Key::A) == 0x41);
    assert(static_cast<uint32_t>(Key::Z) == 0x5A);

    // 功能鍵 (F1 - F24)
    assert(static_cast<uint32_t>(Key::F1) == 0x70);
    assert(static_cast<uint32_t>(Key::F12) == 0x7B);
    assert(static_cast<uint32_t>(Key::F24) == 0x87);

    // 系統鍵
    assert(static_cast<uint32_t>(Key::LWin) == 0x5B);
    assert(static_cast<uint32_t>(Key::RWin) == 0x5C);

    std::cout << "[PASS] TestKeyEnumValues" << std::endl;
}

static void TestKeyClassifiers() {
    // 1. 修飾鍵檢驗
    assert(IsModifierKey(Key::Shift));
    assert(IsModifierKey(Key::Control));
    assert(IsModifierKey(Key::Alt));
    assert(IsModifierKey(Key::LWin));
    assert(IsModifierKey(Key::RWin));
    assert(IsModifierKey(Key::LShift));
    assert(!IsModifierKey(Key::A));
    assert(!IsModifierKey(Key::Enter));
    assert(!IsModifierKey(Key::Space));

    // 2. 箭頭鍵檢驗
    assert(IsArrowKey(Key::Left));
    assert(IsArrowKey(Key::Up));
    assert(IsArrowKey(Key::Right));
    assert(IsArrowKey(Key::Down));
    assert(!IsArrowKey(Key::Home));
    assert(!IsArrowKey(Key::PageUp));
    assert(!IsArrowKey(Key::W));

    // 3. 功能鍵檢驗
    assert(IsFunctionKey(Key::F1));
    assert(IsFunctionKey(Key::F12));
    assert(IsFunctionKey(Key::F24));
    assert(!IsFunctionKey(Key::Escape));
    assert(!IsFunctionKey(Key::D1));
    assert(!IsFunctionKey(Key::A));

    // 4. 英數字鍵檢驗
    assert(IsAlphaNumeric(Key::A));
    assert(IsAlphaNumeric(Key::M));
    assert(IsAlphaNumeric(Key::Z));
    assert(IsAlphaNumeric(Key::D0));
    assert(IsAlphaNumeric(Key::D9));
    assert(!IsAlphaNumeric(Key::F1));
    assert(!IsAlphaNumeric(Key::Space));
    assert(!IsAlphaNumeric(Key::Return));

    std::cout << "[PASS] TestKeyClassifiers" << std::endl;
}

static void TestKeyToString() {
    assert(KeyToString(Key::None) == "None");
    assert(KeyToString(Key::Backspace) == "Backspace");
    assert(KeyToString(Key::Tab) == "Tab");
    assert(KeyToString(Key::Return) == "Return");
    assert(KeyToString(Key::Enter) == "Return");
    assert(KeyToString(Key::Escape) == "Escape");
    assert(KeyToString(Key::Space) == "Space");
    assert(KeyToString(Key::A) == "A");
    assert(KeyToString(Key::Z) == "Z");
    assert(KeyToString(Key::D0) == "0");
    assert(KeyToString(Key::D9) == "9");
    assert(KeyToString(Key::F1) == "F1");
    assert(KeyToString(Key::F12) == "F12");
    assert(KeyToString(Key::Delete) == "Delete");
    assert(KeyToString(Key::Up) == "Up");
    assert(KeyToString(Key::Down) == "Down");
    assert(KeyToString(Key::Left) == "Left");
    assert(KeyToString(Key::Right) == "Right");
    assert(KeyToString(static_cast<Key>(0xFFFF)) == "Unknown");

    std::cout << "[PASS] TestKeyToString" << std::endl;
}

static void TestKeyEventArgsKeyFidelity() {
    const RoutedEvent dummyEvent(1, "KeyDown", RoutingStrategy::Bubble);

    // 1. 使用強型別 Key 建構
    KeyEventArgs args1(dummyEvent, nullptr, Key::Enter, 0, false, pal::KeyModifiers::Control);
    assert(args1.GetKey() == Key::Enter);
    assert(args1.GetVirtualKey() == 0x0D);
    assert(args1.HasControl());
    assert(!args1.HasShift());

    // 2. 使用原始整數 virtualKey 建構
    KeyEventArgs args2(dummyEvent, nullptr, static_cast<uint32_t>(0x41), 0, false, pal::KeyModifiers::Shift);
    assert(args2.GetKey() == Key::A);
    assert(args2.GetVirtualKey() == 0x41);
    assert(args2.HasShift());

    // 3. 雙向轉換工具函式檢驗
    assert(ToVirtualKey(Key::Space) == 0x20);
    assert(FromVirtualKey(0x20) == Key::Space);
    assert(ToVirtualKey(Key::Escape) == 0x1B);
    assert(FromVirtualKey(0x1B) == Key::Escape);

    std::cout << "[PASS] TestKeyEventArgsKeyFidelity" << std::endl;
}

int main() {
    std::cout << "Running Key & Keyboard Codes Unit Tests..." << std::endl;
    TestKeyEnumValues();
    TestKeyClassifiers();
    TestKeyToString();
    TestKeyEventArgsKeyFidelity();
    std::cout << "All Key Unit Tests PASSED Successfully!" << std::endl;
    return 0;
}
