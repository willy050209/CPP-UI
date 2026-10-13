/**
 * @file treeview_test.cpp
 * @brief 樹狀檢視控制項 (TreeView, TreeViewItem, 延遲載入) 單元測試
 */

#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include "compat/compat.hpp"
#include "ui/controls/TreeView.hpp"

using namespace cppui::controls;

static void TestTreeViewBasicHierarchy() {
    TreeView tree;
    assert(tree.GetItems().empty());
    assert(tree.GetSelectedItem() == nullptr);

    auto root1 = tree.AddRootItem("Documents");
    auto root2 = tree.AddRootItem("Pictures");
    assert(tree.GetItems().size() == 2);

    auto sub1 = root1->AddItem("Work");
    auto sub2 = root1->AddItem("Personal");
    assert(root1->HasChildren());
    assert(!root2->HasChildren());
    assert(root1->GetItems().size() == 2);
    assert(sub1->GetParent() == root1.get());

    // 初始收合狀態：僅 2 個根節點可見
    auto flat = tree.GetFlattenedVisibleItems();
    assert(flat.size() == 2);
    assert(flat[0].level == 0);
    assert(flat[0].item == root1.get());
    assert(flat[1].level == 0);
    assert(flat[1].item == root2.get());

    // 展開 root1：可見節點變為 Documents, Work, Personal, Pictures (4 個)
    root1->SetExpanded(true);
    assert(root1->IsExpanded());
    flat = tree.GetFlattenedVisibleItems();
    assert(flat.size() == 4);
    assert(flat[1].level == 1);
    assert(flat[1].item == sub1.get());
    assert(flat[2].level == 1);
    assert(flat[2].item == sub2.get());
    assert(flat[3].level == 0);
    assert(flat[3].item == root2.get());

    // 選取節點
    TreeViewItem* notifiedItem = nullptr;
    tree.OnSelectedItemChanged([&notifiedItem](TreeViewItem* item) {
        notifiedItem = item;
    });

    tree.SetSelectedItem(sub1.get());
    assert(tree.GetSelectedItem() == sub1.get());
    assert(notifiedItem == sub1.get());

    // 收合 root1
    root1->SetExpanded(false);
    assert(!root1->IsExpanded());
    flat = tree.GetFlattenedVisibleItems();
    assert(flat.size() == 2);

    std::cout << "[PASS] TestTreeViewBasicHierarchy" << std::endl;
}

static void TestTreeViewLazyLoading() {
    TreeView tree;
    auto lazyRoot = tree.AddRootItem("Remote Drive");
    assert(!lazyRoot->HasChildren());

    bool handlerInvoked = false;
    lazyRoot->SetExpandingHandler([&handlerInvoked](TreeViewItem& item) {
        handlerInvoked = true;
        item.AddItem("Folder A");
        item.AddItem("Folder B");
    });
    assert(lazyRoot->HasExpandingHandler());

    // 觸發展開，延遲載入回呼應被呼叫並注入子節點
    lazyRoot->SetExpanded(true);
    assert(handlerInvoked);
    assert(lazyRoot->HasChildren());
    assert(lazyRoot->GetItems().size() == 2);

    auto flat = tree.GetFlattenedVisibleItems();
    assert(flat.size() == 3); // Remote Drive, Folder A, Folder B

    std::cout << "[PASS] TestTreeViewLazyLoading" << std::endl;
}

static void TestTreeViewFailFast() {
    TreeView tree;
    bool caught1 = false;
    try {
        tree.AddRootItem(std::shared_ptr<TreeViewItem>(nullptr));
    } catch (const std::invalid_argument&) {
        caught1 = true;
    }
    assert(caught1);

    auto root = tree.AddRootItem("Root");
    bool caught2 = false;
    try {
        root->AddItem(std::shared_ptr<TreeViewItem>(nullptr));
    } catch (const std::invalid_argument&) {
        caught2 = true;
    }
    assert(caught2);

    bool caught3 = false;
    try {
        tree.SetItemHeight(0.0f);
    } catch (const std::invalid_argument&) {
        caught3 = true;
    }
    assert(caught3);

    std::cout << "[PASS] TestTreeViewFailFast" << std::endl;
}

int main() {
    TestTreeViewBasicHierarchy();
    TestTreeViewLazyLoading();
    TestTreeViewFailFast();
    std::cout << "All TreeView Tests Passed Successfully!" << std::endl;
    return 0;
}
