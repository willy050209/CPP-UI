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
