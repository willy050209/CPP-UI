/**
 * @file main.cpp
 * @brief CPPUI 官方獨立視覺化表單設計器進入點
 */

#include "DesignerMainWindow.hpp"
#include "ui/app/Application.hpp"
#include "ui/designer/preview/LiveFormRunner.hpp"
#include "ui/designer/preview/CodePreviewWindow.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"
#include "ui/graphics/Bitmap.hpp"
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

    // 支援 --preview-code 引數直接啟動獨立語法高亮代碼預覽視窗
    if (argc > 1 && std::string(argv[1]) == "--preview-code") {
        auto codeWin = std::make_shared<cppui::designer::preview::CodePreviewWindow>(&mainWindow->GetDocument());
        return cppui::app::Application::Run(codeWin);
    }

    if (argc > 1 && std::string(argv[1]) == "--dump-layout") {
        std::string outPath = (argc > 2) ? argv[2] : "designer_dump.bmp";
        if (argc > 3 && std::string(argv[3]) == "--resize") {
            mainWindow->SetFormSize(1280, 729);
        }
        auto bmp = mainWindow->CaptureToBitmap();
        if (bmp) {
            bool ok = bmp->Save(outPath);
            compat::println("Captured to {}: {}", outPath, ok ? "SUCCESS" : "FAILED");
        }
        return 0;
    }

    return cppui::app::Application::Run(mainWindow);
}
