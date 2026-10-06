# CPPUI 文件中心 (Documentation Hub)

歡迎來到 CPPUI 的文件中心。此處整合了框架的核心設計規範、演進藍圖與完整的 API 參考文檔。

---

## 📑 快速導覽 (Quick Navigation)

### 1. 官方 API 參考手冊 (MSDN-Style API Reference)
* [📖 **CPPUI API 參考文件首頁**](api/README.md)
  * [幾何與核心型別 (`cppui::core`)](api/namespaces/cppui_core.md)
  * [平台抽象層 (`cppui::pal`)](api/namespaces/cppui_pal.md)
  * [繪圖與命令錄製管線 (`cppui::rendering`)](api/namespaces/cppui_rendering.md)
  * [向下相容性與微架構巨集 (`compat`)](api/namespaces/compat.md)
    * [視圖適配器與惰性管道 (`compat::views`)](api/namespaces/compat_views.md)
    * [受約束範圍與演算法 (`compat::ranges`)](api/namespaces/compat_ranges.md)
    * [期望值、可空值與錯誤處理 (`compat::expected`)](api/namespaces/compat_expected.md)
    * [格式化、終端與數值解析 (`compat::format/print`)](api/namespaces/compat_format_print.md)
  * [🚀 快速入門指南 (Quickstart Guide)](api/guides/quickstart.md)
  * [⚙️ DisplayList 二進位錄製深度解析](api/guides/display_list_recording.md)
  * [🖥️ 高分屏 DPI 與座標轉換指南](api/guides/dpi_and_coordinates.md)

---

### 2. 架構設計規範 (Architecture Specifications)
* [01. 三層架構分離與依賴邊界規範](specs/01_architecture_layers.md)
* [02. 代碼規範與向下相容標準 (compat.hpp)](specs/02_coding_standards_compat.md)
* [03. 命令錄製、DisplayList 與記憶體架構](specs/03_rendering_and_memory.md)
* [04. Fluent RAD API 與路由事件標準](specs/04_fluent_api_and_events.md)

---

### 3. 目標藍圖 (Roadmap)
* [🎯 里程碑演進地圖 (M1 ~ M6)](roadmap/target_map.md)
