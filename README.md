# CPPUI: Modern Cross-Platform C++ UI Framework

CPPUI 是一個極致現代化、高效能且具備極速開發體驗的跨平台 C++ UI 框架。

## 🎯 核心設計架構

* **底層渲染**：Skia 2D GPU 硬體加速渲染管線。
* **文字引擎**：ICU（雙向文字與斷詞）+ HarfBuzz（字形塑形）。
* **視覺與排版**：Avalonia / WPF 風格的保留模式視覺樹與雙階段（`Measure` / `Arrange`）約束排版。
* **開發體驗**：Windows Forms 等級的極速直覺代碼驅動體驗（Code-First），搭配現代 C++ Fluent Builder 與 Lambda 信號掛載。
* **向下相容性**：全面導入 [`include/compat/compat.hpp`](include/compat/compat.hpp)，自動在新編譯器使用 `std` 實作，在舊編譯器無痛降級 polyfill。

---

## 📚 設計規範與文件 (Documentation)

請在開始開發或閱讀源碼前，詳閱以下實體規範檔案：
- [📖 **CPPUI MSDN 風格完整 API 參考文件 (API Reference)**](docs/api/README.md)
- [01. 三層架構分離與依賴邊界規範](docs/specs/01_architecture_layers.md)
- [02. 代碼規範與向下相容標準 (compat.hpp)](docs/specs/02_coding_standards_compat.md)
- [03. 命令錄製、DisplayList 與記憶體架構](docs/specs/03_rendering_and_memory.md)
- [04. Fluent RAD API 與路由事件標準](docs/specs/04_fluent_api_and_events.md)
- [🎯 六大階段目標地圖 (Roadmap)](docs/roadmap/target_map.md)

---

## 📁 目錄結構

```text
CPPUI/
├── CMakeLists.txt              # 頂層 CMake 建置檔案
├── include/
│   ├── compat/                 # 向下相容性 Shim Layer (compat.hpp)
│   └── ui/                     # 框架公共頭文件
│       ├── core/               # 基礎幾何、屬性系統、Visual/UIElement
│       ├── pal/                # 平台抽象層 (Window, RunLoop, Input)
│       ├── rendering/          # DisplayList, LinearArena, DrawingContext
│       ├── text/               # HarfBuzz + ICU 文字排版
│       ├── layout/             # 雙階段佈局管線 (Measure/Arrange)
│       ├── input/              # 路由事件、HitTest、FocusManager
│       └── controls/           # 控制項庫與 Fluent Builder DSL
├── src/                        # 實作代碼
├── docs/                       # 設計規範與目標地圖
├── tests/                      # 單元測試與環境驗證
└── examples/                   # 範例程式
```

---

## 🛠️ 建置需求

- **編譯器支援**：
  - Clang / Clang++ 14+ (推薦)
  - GCC / G++ 11+ (MinGW-w64 UCRT64)
  - MSVC 2019 / 2022 / 2025 (`cl.exe`)
- **建置工具**：
  - CMake 3.20+
  - Ninja 或 MSBuild

---

## 🛡️ 記憶體安全與 AddressSanitizer (ASan)

CPPUI 專案全面支援並預設啟用 **AddressSanitizer (ASan)** 記憶體安全檢測機制：

- **CMake 設定開關**：
  - `-DCPPUI_ENABLE_ASAN=ON` (預設為 `ON`，可透過 `-DCPPUI_ENABLE_ASAN=OFF` 關閉)
- **跨編譯器支援**：
  - **MSVC**：自動套用 `/fsanitize=address` 與 `/INCREMENTAL:NO`
  - **GCC / Clang**：自動套用 `-fsanitize=address -fno-omit-frame-pointer` 與 `-fsanitize=address` 鏈結旗標
- **編譯器警告檢查報告 (Compiler Warnings Status)**：
  - 本專案在 **MSVC (`/W4`)**、**GCC (`-Wall -Wextra -Wpedantic`)** 及 **Clang** 嚴格警告設定下全面重構與清理完畢，達成 **0 Warnings / 0 Errors**。
  - 目前所有內部核心程式碼與標頭檔皆無任何無法消除之警告。
  - *備註 (Windows MSVC ASan 執行提示)*：在 Windows 下若於純終端環境（非 Visual Studio 開發人員命令提示字元）直接執行啟用 ASan 的測試二進位檔，請確保 MSVC 的 ASan runtime DLL (`clang_rt.asan_dbg_dynamic-x86_64.dll` / `clang_rt.asan_dynamic-x86_64.dll`) 所在目錄（例如 `VC\Tools\MSVC\<version>\bin\Hostx64\x64`）已加入至環境變數 `PATH` 中。

