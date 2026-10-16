/**
 * @file main.cpp
 * @brief CPPUI 官方獨立視覺化表單設計器進入點
 */

#include "DesignerMainWindow.hpp"
#include "ui/app/Application.hpp"
#include "ui/designer/preview/LiveFormRunner.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"
#include <memory>
#include <string>

int main(int argc, char* argv[]) {
    auto mainWindow = std::make_shared<cppui::designer::tool::DesignerMainWindow>();

    // 支援 --live-run 引數直接啟動真實表單試運行視窗 (方便截圖與 CI 自動化驗證)
    if (argc > 1 && std::string(argv[1]) == "--live-run") {
        auto liveForm = cppui::designer::preview::LiveFormRunner::CreateForm(mainWindow->GetDocument());
        if (liveForm) {
            liveForm->SetTitle("【試運行 (Live Run)】" + mainWindow->GetDocument().GetSettings().className);
            return cppui::app::Application::Run(liveForm);
        }
    }

    return cppui::app::Application::Run(mainWindow);
}
