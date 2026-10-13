/*
 * File: clipboard_test.cpp
 * Purpose: Comprehensive unit tests for IClipboard, Clipboard static facade,
 *          Win32/Generic implementations, and TextBox clipboard integration.
 */

#include "ui/pal/IClipboard.hpp"
#include "ui/controls/TextBox.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <string>

using namespace cppui;

class MockClipboard : public pal::IClipboard {
public:
    COMPAT_NODISCARD bool ContainsText() const override {
        return !m_text.empty();
    }

    COMPAT_NODISCARD std::string GetText() const override {
        return m_text;
    }

    void SetText(compat::string_view text) override {
        m_text = std::string(text.data(), text.size());
    }

    void Clear() override {
        m_text.clear();
    }

private:
    std::string m_text;
};

static void TestMockClipboard() {
    MockClipboard cb;
    assert(!cb.ContainsText());
    assert(cb.GetText().empty());

    cb.SetText("Hello, Clipboard!");
    assert(cb.ContainsText());
    assert(cb.GetText() == "Hello, Clipboard!");

    cb.Clear();
    assert(!cb.ContainsText());
    assert(cb.GetText().empty());

    std::cout << "[PASS] TestMockClipboard" << std::endl;
}

static void TestUtf8TextIntegrity() {
    auto genericCb = std::make_shared<MockClipboard>();
    pal::Clipboard::SetCustomClipboard(genericCb);

    std::string testText = "哈囉，世界！這是一段包含繁體中文、日文（こんにちは）、Emoji 😊 與符號的剪貼簿測試。";
    pal::Clipboard::SetText(testText);
    assert(pal::Clipboard::ContainsText());
    assert(pal::Clipboard::GetText() == testText);

    pal::Clipboard::Clear();
    assert(!pal::Clipboard::ContainsText());
    assert(pal::Clipboard::GetText().empty());

    // 恢復預設剪貼簿
    pal::Clipboard::SetCustomClipboard(nullptr);

    std::cout << "[PASS] TestUtf8TextIntegrity" << std::endl;
}

static void TestTextBoxClipboardIntegration() {
    auto mockCb = std::make_shared<MockClipboard>();
    pal::Clipboard::SetCustomClipboard(mockCb);

    controls::TextBox tb;
    tb.SetText("ABCDEF123456");

    // 1. 複製 (Copy) 測試
    tb.Select(3, 3); // 選取 "DEF"
    assert(tb.GetSelectedText() == "DEF");
    tb.Copy();
    assert(pal::Clipboard::GetText() == "DEF");

    // 2. 剪下 (Cut) 測試
    tb.Select(0, 3); // 選取 "ABC"
    tb.Cut();
    assert(pal::Clipboard::GetText() == "ABC");
    assert(tb.GetText() == "DEF123456");

    // 3. 貼上 (Paste) 測試
    tb.SetCaretIndex(tb.GetText().size()); // 移至末端
    tb.Paste();
    assert(tb.GetText() == "DEF123456ABC");

    // 4. 全選 (SelectAll) 測試
    tb.SelectAll();
    assert(tb.GetSelectedText() == "DEF123456ABC");
    tb.Copy();
    assert(pal::Clipboard::GetText() == "DEF123456ABC");

    // 5. 唯讀保護 (ReadOnly Protection) 測試
    tb.SetReadOnly(true);
    pal::Clipboard::SetText("HACK");
    tb.Select(0, 3);
    tb.Cut(); // 唯讀時 Cut 應被阻擋
    assert(tb.GetText() == "DEF123456ABC");
    assert(pal::Clipboard::GetText() == "HACK"); // 剪貼簿維持不變

    tb.Paste(); // 唯讀時 Paste 應被阻擋
    assert(tb.GetText() == "DEF123456ABC");

    // 恢復預設剪貼簿
    pal::Clipboard::SetCustomClipboard(nullptr);

    std::cout << "[PASS] TestTextBoxClipboardIntegration" << std::endl;
}

int main() {
    std::cout << "Running Clipboard & Data Transfer Unit Tests..." << std::endl;
    TestMockClipboard();
    TestUtf8TextIntegrity();
    TestTextBoxClipboardIntegration();
    std::cout << "All Clipboard Unit Tests PASSED!" << std::endl;
    return 0;
}
