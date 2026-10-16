/**
 * @file designer_command_undo_test.cpp
 * @brief Undo/Redo 命令狀態機、子樹保留、命令合併與 500 步隨機 Fuzzing 壓力測試
 */

#include <cassert>
#include <iostream>
#include <random>
#include <thread>
#include "compat/Compat.hpp"
#include "ui/Designer.hpp"

using namespace cppui::designer;

static void TestAtomicCommandUndoRedo() {
    compat::println("--- Running TestAtomicCommandUndoRedo ---");

    FormDocumentNode doc;
    auto rootGrid = std::make_unique<WidgetNode>(NodeType::Grid, "rootGrid");
    std::string rootUuid = rootGrid->GetUuid();
    doc.SetRoot(std::move(rootGrid));

    CommandHistoryManager history;

    // 1. AddWidgetCommand
    auto btn = std::make_unique<WidgetNode>(NodeType::Button, "button1");
    std::string btnUuid = btn->GetUuid();
    auto addCmd = std::make_unique<AddWidgetCommand>(rootUuid, std::move(btn));
    assert(history.Execute(doc, std::move(addCmd)).has_value());
    assert(doc.GetRoot()->GetChildCount() == 1);
    assert(doc.FindByUuid(btnUuid) != nullptr);

    // 2. SetPropertyCommand
    auto setPropCmd = std::make_unique<SetPropertyCommand>(btnUuid, "Width", PropertyValue(150.0));
    assert(history.Execute(doc, std::move(setPropCmd)).has_value());
    assert(doc.FindByUuid(btnUuid)->GetProperty("Width")->Get<double>().value() == 150.0);

    // 3. Undo SetPropertyCommand
    assert(history.CanUndo());
    assert(history.Undo(doc).has_value());
    assert(!doc.FindByUuid(btnUuid)->GetProperty("Width").has_value());

    // 4. Redo SetPropertyCommand
    assert(history.CanRedo());
    assert(history.Redo(doc).has_value());
    assert(doc.FindByUuid(btnUuid)->GetProperty("Width")->Get<double>().value() == 150.0);

    // 5. RemoveWidgetCommand
    auto rmCmd = std::make_unique<RemoveWidgetCommand>(rootUuid, btnUuid);
    assert(history.Execute(doc, std::move(rmCmd)).has_value());
    assert(doc.GetRoot()->GetChildCount() == 0);
    assert(doc.FindByUuid(btnUuid) == nullptr);

    // 6. Undo RemoveWidgetCommand (深層子樹與原始 UUID 必須完整還原)
    assert(history.Undo(doc).has_value());
    assert(doc.GetRoot()->GetChildCount() == 1);
    WidgetNode* restoredBtn = doc.FindByUuid(btnUuid);
    assert(restoredBtn != nullptr);
    assert(restoredBtn->GetProperty("Width")->Get<double>().value() == 150.0);

    // 7. 全部 Undo 回到最初空白狀態
    assert(history.Undo(doc).has_value()); // undo SetProperty
    assert(history.Undo(doc).has_value()); // undo AddWidget
    assert(doc.GetRoot()->GetChildCount() == 0);
    assert(!history.CanUndo());
}

static void TestMacroCommand() {
    compat::println("--- Running TestMacroCommand ---");

    FormDocumentNode doc;
    auto rootGrid = std::make_unique<WidgetNode>(NodeType::Grid, "rootGrid");
    std::string rootUuid = rootGrid->GetUuid();
    doc.SetRoot(std::move(rootGrid));

    CommandHistoryManager history;

    // 打包批次新增 3 個按鈕為單一 MacroCommand
    auto macro = std::make_unique<MacroCommand>("批次新增按鈕");
    auto b1 = std::make_unique<WidgetNode>(NodeType::Button, "btn1");
    auto b2 = std::make_unique<WidgetNode>(NodeType::Button, "btn2");
    auto b3 = std::make_unique<WidgetNode>(NodeType::Button, "btn3");

    std::string b1Uuid = b1->GetUuid();
    std::string b2Uuid = b2->GetUuid();
    std::string b3Uuid = b3->GetUuid();

    macro->AddCommand(std::make_unique<AddWidgetCommand>(rootUuid, std::move(b1)));
    macro->AddCommand(std::make_unique<AddWidgetCommand>(rootUuid, std::move(b2)));
    macro->AddCommand(std::make_unique<AddWidgetCommand>(rootUuid, std::move(b3)));

    assert(history.Execute(doc, std::move(macro)).has_value());
    assert(doc.GetRoot()->GetChildCount() == 3);

    // 單次 Undo 必須一次性還原整批 3 個按鈕
    assert(history.Undo(doc).has_value());
    assert(doc.GetRoot()->GetChildCount() == 0);

    // 單次 Redo 必須一次性重新加入 3 個按鈕
    assert(history.Redo(doc).has_value());
    assert(doc.GetRoot()->GetChildCount() == 3);
    assert(doc.FindByUuid(b1Uuid) != nullptr);
    assert(doc.FindByUuid(b2Uuid) != nullptr);
    assert(doc.FindByUuid(b3Uuid) != nullptr);
}

static void TestCommandCoalescing() {
    compat::println("--- Running TestCommandCoalescing ---");

    FormDocumentNode doc;
    auto rootGrid = std::make_unique<WidgetNode>(NodeType::Grid, "rootGrid");
    std::string rootUuid = rootGrid->GetUuid();
    doc.SetRoot(std::move(rootGrid));

    CommandHistoryManager history;

    auto slider = std::make_unique<WidgetNode>(NodeType::Slider, "slider1");
    std::string sliderUuid = slider->GetUuid();
    assert(history.Execute(doc, std::make_unique<AddWidgetCommand>(rootUuid, std::move(slider))).has_value());

    // 連續 50 次微小拖曳數值變更 (間隔極短，觸發 Coalescing 合併)
    for (int32_t i = 1; i <= 50; ++i) {
        auto cmd = std::make_unique<SetPropertyCommand>(sliderUuid, "Value", PropertyValue(static_cast<double>(i)));
        assert(history.Execute(doc, std::move(cmd)).has_value());
    }

    // 由於所有 50 次操作在 300ms 閾值內連續發生，歷史記錄中應合併為 1 個 SetProperty 命令
    // (總歷史 = 1 個 Add + 1 個合併的 SetProperty = 2)
    assert(history.GetUndoCount() == 2);
    assert(doc.FindByUuid(sliderUuid)->GetProperty("Value")->Get<double>().value() == 50.0);

    // 單次 Undo 直接退回最原始數值 (尚未設定 Value 屬性)
    assert(history.Undo(doc).has_value());
    assert(!doc.FindByUuid(sliderUuid)->GetProperty("Value").has_value());

    // 單次 Redo 直接恢復到最終數值 50.0
    assert(history.Redo(doc).has_value());
    assert(doc.FindByUuid(sliderUuid)->GetProperty("Value")->Get<double>().value() == 50.0);
}

static void TestSavePointAndDirtyTracking() {
    compat::println("--- Running TestSavePointAndDirtyTracking ---");

    FormDocumentNode doc;
    auto rootGrid = std::make_unique<WidgetNode>(NodeType::Grid, "rootGrid");
    std::string rootUuid = rootGrid->GetUuid();
    doc.SetRoot(std::move(rootGrid));

    CommandHistoryManager history;
    assert(!history.IsDirty());

    // 執行操作 -> 變為 Dirty
    auto btn = std::make_unique<WidgetNode>(NodeType::Button, "btn");
    std::string btnUuid = btn->GetUuid();
    assert(history.Execute(doc, std::make_unique<AddWidgetCommand>(rootUuid, std::move(btn))).has_value());
    assert(history.IsDirty());

    // 存檔 (SetSavePoint) -> 恢復為 Not Dirty
    history.SetSavePoint();
    assert(!history.IsDirty());

    // 再次修改 -> 變為 Dirty
    assert(history.Execute(doc, std::make_unique<SetPropertyCommand>(btnUuid, "Width", PropertyValue(100.0))).has_value());
    assert(history.IsDirty());

    // Undo 回到存檔點 -> 自動變回 Not Dirty!
    assert(history.Undo(doc).has_value());
    assert(!history.IsDirty());

    // Redo 離開存檔點 -> 再次變為 Dirty
    assert(history.Redo(doc).has_value());
    assert(history.IsDirty());
}

static void TestFuzzing500StepsStress() {
    compat::println("--- Running TestFuzzing500StepsStress (500-step randomized stress test) ---");

    FormDocumentNode doc;
    auto rootGrid = std::make_unique<WidgetNode>(NodeType::Grid, "rootGrid");
    std::string rootUuid = rootGrid->GetUuid();
    doc.SetRoot(std::move(rootGrid));

    CommandHistoryManager history(1000); // 擴大堆疊以容納 500 步

    std::mt19937 rng(12345); // 固定種子確保可重複性
    std::vector<std::string> activeUuids;

    // 執行 500 步隨機操作序列
    for (int32_t step = 0; step < 500; ++step) {
        int32_t op = rng() % 3;

        if (op == 0 || activeUuids.empty()) {
            // 新增按鈕或文字塊
            NodeType t = (rng() % 2 == 0) ? NodeType::Button : NodeType::TextBlock;
            auto node = std::make_unique<WidgetNode>(t, "node_" + std::to_string(step));
            std::string u = node->GetUuid();
            auto cmd = std::make_unique<AddWidgetCommand>(rootUuid, std::move(node));
            if (history.Execute(doc, std::move(cmd)).has_value()) {
                activeUuids.push_back(u);
            }
        } else if (op == 1 && !activeUuids.empty()) {
            // 修改隨機節點屬性
            size_t idx = rng() % activeUuids.size();
            std::string u = activeUuids[idx];
            double w = static_cast<double>((rng() % 500) + 10);
            auto cmd = std::make_unique<SetPropertyCommand>(u, "Width", PropertyValue(w));
            (void)history.Execute(doc, std::move(cmd));
        } else if (op == 2 && activeUuids.size() > 1) {
            // 刪除隨機節點
            size_t idx = rng() % activeUuids.size();
            std::string u = activeUuids[idx];
            auto cmd = std::make_unique<RemoveWidgetCommand>(rootUuid, u);
            if (history.Execute(doc, std::move(cmd)).has_value()) {
                activeUuids.erase(activeUuids.begin() + idx);
            }
        }
    }

    size_t totalStepsExecuted = history.GetUndoCount();
    assert(totalStepsExecuted > 0);

    // 全部撤銷至初始狀態
    for (size_t i = 0; i < totalStepsExecuted; ++i) {
        assert(history.CanUndo());
        assert(history.Undo(doc).has_value());
    }
    assert(!history.CanUndo());
    assert(doc.GetRoot()->GetChildCount() == 0); // 初始狀態子節點必須為 0

    // 全部重做至最終狀態
    for (size_t i = 0; i < totalStepsExecuted; ++i) {
        assert(history.CanRedo());
        assert(history.Redo(doc).has_value());
    }
    assert(!history.CanRedo());
    assert(doc.GetRoot()->GetChildCount() == activeUuids.size());
}

int main() {
    compat::println("==================================================");
    compat::println("Starting designer_command_undo_test...");
    compat::println("==================================================");

    TestAtomicCommandUndoRedo();
    TestMacroCommand();
    TestCommandCoalescing();
    TestSavePointAndDirtyTracking();
    TestFuzzing500StepsStress();

    compat::println("All designer_command_undo_test passed successfully!");
    return 0;
}
