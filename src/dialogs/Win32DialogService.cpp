/**
 * @file Win32DialogService.cpp
 * @brief 實作 Win32 原生 COM 對話框服務 (IFileOpenDialog, IFileSaveDialog, MessageBoxW)
 */

#include "Win32DialogService.hpp"
#include "ui/app/Form.hpp"
#include "ui/pal/IPlatformWindow.hpp"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shobjidl.h>
#include <shlwapi.h>

namespace cppui {
namespace dialogs {

namespace {

// UTF-8 轉 Wide String
static std::wstring Utf8ToWide(compat::string_view utf8) {
    if (utf8.empty()) return std::wstring();
    int32_t len = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int32_t>(utf8.size()), nullptr, 0);
    if (len <= 0) return std::wstring();
    std::wstring result(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int32_t>(utf8.size()), &result[0], len);
    return result;
}

// Wide String 轉 UTF-8
static std::string WideToUtf8(const wchar_t* wide) {
    if (!wide || *wide == L'\0') return std::string();
    int32_t len = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 1) return std::string();
    std::string result(static_cast<size_t>(len - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, &result[0], len - 1, nullptr, nullptr);
    return result;
}

// RAII COM 初始化守護
struct ScopedCom {
    HRESULT hr;
    ScopedCom() {
        hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    }
    ~ScopedCom() {
        if (SUCCEEDED(hr)) {
            CoUninitialize();
        }
    }
};

static HWND ExtractHwnd(const std::shared_ptr<app::Form>& owner) {
    if (owner && owner->GetPlatformWindow()) {
        return static_cast<HWND>(owner->GetPlatformWindow()->GetNativeHandle());
    }
    return nullptr;
}

} // namespace

bool Win32DialogService::IsAvailable() noexcept {
    return true;
}

app::DialogResult Win32DialogService::ShowMessageBox(
    std::shared_ptr<app::Form> owner,
    compat::string_view text,
    compat::string_view caption,
    MessageBoxButtons buttons,
    MessageBoxIcon icon,
    MessageBoxDefaultButton defaultButton) {

    std::wstring wText = Utf8ToWide(text);
    std::wstring wCaption = Utf8ToWide(caption.empty() ? "訊息" : caption);

    UINT uType = 0;

    // 按鈕對應
    switch (buttons) {
    case MessageBoxButtons::OK:
        uType |= MB_OK;
        break;
    case MessageBoxButtons::OKCancel:
        uType |= MB_OKCANCEL;
        break;
    case MessageBoxButtons::YesNo:
        uType |= MB_YESNO;
        break;
    case MessageBoxButtons::YesNoCancel:
        uType |= MB_YESNOCANCEL;
        break;
    case MessageBoxButtons::RetryCancel:
        uType |= MB_RETRYCANCEL;
        break;
    case MessageBoxButtons::AbortRetryIgnore:
        uType |= MB_ABORTRETRYIGNORE;
        break;
    }

    // 圖示對應
    switch (icon) {
    case MessageBoxIcon::Information:
        uType |= MB_ICONINFORMATION;
        break;
    case MessageBoxIcon::Warning:
        uType |= MB_ICONWARNING;
        break;
    case MessageBoxIcon::Error:
        uType |= MB_ICONERROR;
        break;
    case MessageBoxIcon::Question:
        uType |= MB_ICONQUESTION;
        break;
    case MessageBoxIcon::None:
    default:
        break;
    }

    // 預設按鈕
    switch (defaultButton) {
    case MessageBoxDefaultButton::Button1:
        uType |= MB_DEFBUTTON1;
        break;
    case MessageBoxDefaultButton::Button2:
        uType |= MB_DEFBUTTON2;
        break;
    case MessageBoxDefaultButton::Button3:
        uType |= MB_DEFBUTTON3;
        break;
    }

    HWND hwnd = ExtractHwnd(owner);
    int32_t ret = ::MessageBoxW(hwnd, wText.c_str(), wCaption.c_str(), uType);

    switch (ret) {
    case IDOK:     return app::DialogResult::OK;
    case IDCANCEL: return app::DialogResult::Cancel;
    case IDYES:    return app::DialogResult::Yes;
    case IDNO:     return app::DialogResult::No;
    case IDRETRY:  return app::DialogResult::Retry;
    case IDABORT:  return app::DialogResult::Abort;
    case IDIGNORE: return app::DialogResult::Ignore;
    default:       return app::DialogResult::None;
    }
}

app::DialogResult Win32DialogService::ShowOpenFileDialog(
    std::shared_ptr<app::Form> owner,
    compat::string_view title,
    compat::string_view initialDir,
    compat::string_view filter,
    uint32_t filterIndex,
    compat::string_view defaultExt,
    bool multiselect,
    bool checkFileExists,
    bool checkPathExists,
    std::string& outSelectedFile,
    std::vector<std::string>& outSelectedFiles) {

    ScopedCom com;
    IFileOpenDialog* pFileOpen = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
    if (FAILED(hr) || !pFileOpen) {
        return app::DialogResult::Cancel;
    }

    // 設定選項旗標
    FILEOPENDIALOGOPTIONS opt = 0;
    pFileOpen->GetOptions(&opt);
    opt |= FOS_FORCEFILESYSTEM;
    if (checkFileExists) opt |= FOS_FILEMUSTEXIST;
    if (checkPathExists) opt |= FOS_PATHMUSTEXIST;
    if (multiselect) opt |= FOS_ALLOWMULTISELECT;
    pFileOpen->SetOptions(opt);

    // 標題
    if (!title.empty()) {
        std::wstring wTitle = Utf8ToWide(title);
        pFileOpen->SetTitle(wTitle.c_str());
    }

    // 預設副檔名
    if (!defaultExt.empty()) {
        std::wstring wExt = Utf8ToWide(defaultExt);
        if (!wExt.empty() && wExt[0] == L'.') {
            pFileOpen->SetDefaultExtension(wExt.c_str() + 1);
        } else {
            pFileOpen->SetDefaultExtension(wExt.c_str());
        }
    }

    // 起始目錄
    if (!initialDir.empty()) {
        std::wstring wDir = Utf8ToWide(initialDir);
        IShellItem* pFolder = nullptr;
        if (SUCCEEDED(SHCreateItemFromParsingName(wDir.c_str(), nullptr, IID_PPV_ARGS(&pFolder)))) {
            pFileOpen->SetFolder(pFolder);
            pFolder->Release();
        }
    }

    // 解析篩選器規格
    auto filterItems = DialogUtilities::ParseFilterString(filter);
    std::vector<std::wstring> nameHolders;
    std::vector<std::wstring> specHolders;
    std::vector<COMDLG_FILTERSPEC> specs;

    nameHolders.reserve(filterItems.size());
    specHolders.reserve(filterItems.size());
    specs.reserve(filterItems.size());

    for (const auto& item : filterItems) {
        nameHolders.push_back(Utf8ToWide(item.description));
        std::string specCombined;
        for (size_t k = 0; k < item.patterns.size(); ++k) {
            if (k > 0) specCombined.push_back(';');
            specCombined.append(item.patterns[k]);
        }
        specHolders.push_back(Utf8ToWide(specCombined));

        COMDLG_FILTERSPEC fs;
        fs.pszName = nameHolders.back().c_str();
        fs.pszSpec = specHolders.back().c_str();
        specs.push_back(fs);
    }

    if (!specs.empty()) {
        pFileOpen->SetFileTypes(static_cast<UINT>(specs.size()), specs.data());
        UINT idx = (filterIndex > 0 && filterIndex <= specs.size()) ? filterIndex : 1;
        pFileOpen->SetFileTypeIndex(idx);
    }

    HWND hwnd = ExtractHwnd(owner);
    hr = pFileOpen->Show(hwnd);

    if (SUCCEEDED(hr)) {
        outSelectedFiles.clear();
        if (multiselect) {
            IShellItemArray* pItems = nullptr;
            if (SUCCEEDED(pFileOpen->GetResults(&pItems))) {
                DWORD count = 0;
                pItems->GetCount(&count);
                for (DWORD i = 0; i < count; ++i) {
                    IShellItem* pItem = nullptr;
                    if (SUCCEEDED(pItems->GetItemAt(i, &pItem))) {
                        PWSTR pszPath = nullptr;
                        if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
                            std::string pathUtf8 = WideToUtf8(pszPath);
                            outSelectedFiles.push_back(pathUtf8);
                            CoTaskMemFree(pszPath);
                        }
                        pItem->Release();
                    }
                }
                pItems->Release();
            }
            if (!outSelectedFiles.empty()) {
                outSelectedFile = outSelectedFiles[0];
            }
        } else {
            IShellItem* pItem = nullptr;
            if (SUCCEEDED(pFileOpen->GetResult(&pItem))) {
                PWSTR pszPath = nullptr;
                if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
                    outSelectedFile = WideToUtf8(pszPath);
                    outSelectedFiles.push_back(outSelectedFile);
                    CoTaskMemFree(pszPath);
                }
                pItem->Release();
            }
        }
        pFileOpen->Release();
        return app::DialogResult::OK;
    }

    pFileOpen->Release();
    return app::DialogResult::Cancel;
}

app::DialogResult Win32DialogService::ShowSaveFileDialog(
    std::shared_ptr<app::Form> owner,
    compat::string_view title,
    compat::string_view initialDir,
    compat::string_view fileName,
    compat::string_view filter,
    uint32_t filterIndex,
    compat::string_view defaultExt,
    bool overwritePrompt,
    bool checkPathExists,
    std::string& outSelectedFile) {

    ScopedCom com;
    IFileSaveDialog* pFileSave = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));
    if (FAILED(hr) || !pFileSave) {
        return app::DialogResult::Cancel;
    }

    FILEOPENDIALOGOPTIONS opt = 0;
    pFileSave->GetOptions(&opt);
    opt |= FOS_FORCEFILESYSTEM;
    if (overwritePrompt) opt |= FOS_OVERWRITEPROMPT;
    if (checkPathExists) opt |= FOS_PATHMUSTEXIST;
    pFileSave->SetOptions(opt);

    if (!title.empty()) {
        std::wstring wTitle = Utf8ToWide(title);
        pFileSave->SetTitle(wTitle.c_str());
    }

    if (!fileName.empty()) {
        std::wstring wName = Utf8ToWide(fileName);
        pFileSave->SetFileName(wName.c_str());
    }

    if (!defaultExt.empty()) {
        std::wstring wExt = Utf8ToWide(defaultExt);
        if (!wExt.empty() && wExt[0] == L'.') {
            pFileSave->SetDefaultExtension(wExt.c_str() + 1);
        } else {
            pFileSave->SetDefaultExtension(wExt.c_str());
        }
    }

    if (!initialDir.empty()) {
        std::wstring wDir = Utf8ToWide(initialDir);
        IShellItem* pFolder = nullptr;
        if (SUCCEEDED(SHCreateItemFromParsingName(wDir.c_str(), nullptr, IID_PPV_ARGS(&pFolder)))) {
            pFileSave->SetFolder(pFolder);
            pFolder->Release();
        }
    }

    auto filterItems = DialogUtilities::ParseFilterString(filter);
    std::vector<std::wstring> nameHolders;
    std::vector<std::wstring> specHolders;
    std::vector<COMDLG_FILTERSPEC> specs;

    nameHolders.reserve(filterItems.size());
    specHolders.reserve(filterItems.size());
    specs.reserve(filterItems.size());

    for (const auto& item : filterItems) {
        nameHolders.push_back(Utf8ToWide(item.description));
        std::string specCombined;
        for (size_t k = 0; k < item.patterns.size(); ++k) {
            if (k > 0) specCombined.push_back(';');
            specCombined.append(item.patterns[k]);
        }
        specHolders.push_back(Utf8ToWide(specCombined));

        COMDLG_FILTERSPEC fs;
        fs.pszName = nameHolders.back().c_str();
        fs.pszSpec = specHolders.back().c_str();
        specs.push_back(fs);
    }

    if (!specs.empty()) {
        pFileSave->SetFileTypes(static_cast<UINT>(specs.size()), specs.data());
        UINT idx = (filterIndex > 0 && filterIndex <= specs.size()) ? filterIndex : 1;
        pFileSave->SetFileTypeIndex(idx);
    }

    HWND hwnd = ExtractHwnd(owner);
    hr = pFileSave->Show(hwnd);

    if (SUCCEEDED(hr)) {
        IShellItem* pItem = nullptr;
        if (SUCCEEDED(pFileSave->GetResult(&pItem))) {
            PWSTR pszPath = nullptr;
            if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
                outSelectedFile = WideToUtf8(pszPath);
                CoTaskMemFree(pszPath);
            }
            pItem->Release();
        }
        pFileSave->Release();
        return app::DialogResult::OK;
    }

    pFileSave->Release();
    return app::DialogResult::Cancel;
}

} // namespace dialogs
} // namespace cppui

#else // !defined(_WIN32)

namespace cppui {
namespace dialogs {

bool Win32DialogService::IsAvailable() noexcept {
    return false;
}

app::DialogResult Win32DialogService::ShowMessageBox(
    std::shared_ptr<app::Form>,
    compat::string_view,
    compat::string_view,
    MessageBoxButtons,
    MessageBoxIcon,
    MessageBoxDefaultButton) {
    return app::DialogResult::None;
}

app::DialogResult Win32DialogService::ShowOpenFileDialog(
    std::shared_ptr<app::Form>,
    compat::string_view,
    compat::string_view,
    compat::string_view,
    uint32_t,
    compat::string_view,
    bool,
    bool,
    bool,
    std::string&,
    std::vector<std::string>&) {
    return app::DialogResult::None;
}

app::DialogResult Win32DialogService::ShowSaveFileDialog(
    std::shared_ptr<app::Form>,
    compat::string_view,
    compat::string_view,
    compat::string_view,
    compat::string_view,
    uint32_t,
    compat::string_view,
    bool,
    bool,
    std::string&) {
    return app::DialogResult::None;
}

} // namespace dialogs
} // namespace cppui

#endif // defined(_WIN32)
