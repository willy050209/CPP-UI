/**
 * @file main.cpp
 * @brief UI 虛擬化與高階展示控制項範例 (ListBox 100k, TreeView, DataGrid, TextBox 剪貼簿)
 */

#include "compat/compat.hpp"
#include "ui/app/AppBuilder.hpp"
#include "ui/app/Form.hpp"
#include "ui/controls/ListBox.hpp"
#include "ui/controls/TreeView.hpp"
#include "ui/controls/DataGrid.hpp"
#include "ui/controls/TextBox.hpp"
#include "ui/controls/Label.hpp"
#include "ui/layout/StackPanel.hpp"
#include <string>

using namespace cppui;
using namespace cppui::controls;
using namespace cppui::layout;

class VirtualizedDemoForm : public app::Form {
public:
    VirtualizedDemoForm() {
        SetTitle("CppUI P2 Showcase: Virtualization, Controls & Clipboard");
        SetWidth(900.0f);
        SetHeight(650.0f);

        auto rootPanel = std::make_shared<StackPanel>();
        rootPanel->SetOrientation(Orientation::Vertical);

        auto lblHeader = std::make_shared<Label>("P2 Showcase: Virtualization & Advanced Controls");
        lblHeader->SetMargin(core::Thickness(12.0f, 12.0f, 12.0f, 6.0f));
        rootPanel->AddChild(lblHeader);

        // 1. TextBox 支援系統剪貼簿
        auto txtInput = std::make_shared<TextBox>();
        txtInput->SetText("Try Ctrl+C, Ctrl+V, or Right-Click here!");
        txtInput->SetWidth(400.0f);
        txtInput->SetHeight(28.0f);
        txtInput->SetMargin(core::Thickness(12.0f, 4.0f, 12.0f, 8.0f));
        rootPanel->AddChild(txtInput);

        auto columnsPanel = std::make_shared<StackPanel>();
        columnsPanel->SetOrientation(Orientation::Horizontal);
        columnsPanel->SetMargin(core::Thickness(12.0f));

        // 2. 虛擬化清單方塊 (100,000 筆資料)
        auto list = std::make_shared<ListBox>();
        list->SetWidth(250.0f);
        list->SetHeight(480.0f);
        list->SetVirtualizationEnabled(true);
        for (size_t i = 1; i <= 100000; ++i) {
            list->AddItem("Virtual Record #" + std::to_string(i));
        }
        columnsPanel->AddChild(list);

        // 3. 階層樹狀檢視 (TreeView)
        auto tree = std::make_shared<TreeView>();
        tree->SetWidth(250.0f);
        tree->SetHeight(480.0f);
        tree->SetMargin(core::Thickness(10.0f, 0.0f, 10.0f, 0.0f));

        auto cRoot = tree->AddRootItem("C: Local Disk");
        auto sys = cRoot->AddItem("Windows");
        sys->AddItem("System32");
        sys->AddItem("Fonts");
        auto prog = cRoot->AddItem("Program Files");
        prog->AddItem("CppUI Framework");

        auto dRoot = tree->AddRootItem("D: Projects");
        dRoot->SetExpandingHandler([](TreeViewItem& item) {
            item.AddItem("Repository Alpha");
            item.AddItem("Repository Beta");
        });
        cRoot->SetExpanded(true);
        columnsPanel->AddChild(tree);

        // 4. 多欄表格控制項 (DataGrid)
        auto grid = std::make_shared<DataGrid>();
        grid->SetWidth(340.0f);
        grid->SetHeight(480.0f);
        grid->AddColumn("ID", DataGridLength::Pixel(50.0f));
        grid->AddColumn("Component", DataGridLength::Star(2.0f));
        grid->AddColumn("Status", DataGridLength::Auto());

        grid->AddRow({ "1", "VirtualizingStackPanel", "Active" });
        grid->AddRow({ "2", "Native Clipboard", "Active" });
        grid->AddRow({ "3", "TreeView Control", "Active" });
        grid->AddRow({ "4", "DataGrid Control", "Active" });
        grid->AddRow({ "5", "DevTools Inspector", "Active" });
        columnsPanel->AddChild(grid);

        rootPanel->AddChild(columnsPanel);
        SetContent(rootPanel);
    }
};

int main() {
    app::AppBuilder builder;
    auto app = builder.Build();
    return app.Run<VirtualizedDemoForm>();
}
