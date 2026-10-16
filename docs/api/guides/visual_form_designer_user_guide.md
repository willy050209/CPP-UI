# CPPUI 視覺化表單設計器官方使用手冊 (Visual Form Designer User Guide)

歡迎使用 **CPPUI 官方獨立視覺化表單設計器 (`cppui-designer`)**。

CPPUI 表單設計器是一套現代化、高效能且與 CPPUI 核心完全解耦的獨立桌面視覺化開發工具。其地位對標 **Qt Designer**、**Visual Studio Windows Forms Designer**、以及 **Android Studio Layout Editor**，旨在消除傳統 C++ 桌面開發中「手寫繁複座標排版」與「繁重 XML/XAML 編譯流程」的雙重痛點。

---

## 📸 設計器介面視覺預覽 (Visual Preview)

![CPPUI Visual Form Designer Preview](../../images/visual_form_designer_preview.png)
*圖 1：CPPUI 視覺化表單設計器完整三欄式工作台*

---

## 一、 核心設計哲學與價值 (Philosophy & Value)

1. **激進的直覺拖曳開發體驗 (Radical RAD)**：
   無需預先編寫繁複程式碼，透過滑鼠點選與拖曳即可完成多層級 Grid、StackPanel、按鈕、輸入框與數據表格的視覺化編排。
2. **零 XML / XAML、零 MOC 執行期反射**：
   完全基於純 C++ 現代領域 AST 模型（`FormDocumentNode` 與 `WidgetNode`），支援雙向即時生成型別安全的 **Fluent DSL** 或 **Classic C++ Code-Behind** 代碼。
3. **現代 Pimpl 深常數性 (Deep Constness)**：
   所有內部狀態封裝於 `compat::propagate_const<std::unique_ptr<Impl>>`，杜絕淺常數性漏洞，提供工業級 ABI 穩定度。
4. **增量使用者程式碼保護 (Incremental User Code Protection)**：
   專案匯出時全量覆寫由設計器維護的 `.designer.hpp`，但對使用者自訂商業邏輯的 `.cpp` 檔案**僅增量追加新事件常式存根**，100% 完整保留使用者的自訂函式、變數與註解。

---

## 二、 工作台介面全覽 (Workspace Tour)

CPPUI 表單設計器主視窗採用專業的三欄式現代深色佈局：

```text
+----------------------------------------------------------------------------------------------------+
| 💠 CPPUI Designer   [↶ 復原] [↷ 重做] [🗑 刪除]   [▶ 預覽執行] [🎨 切換主題] [📦 匯出 CMake 專案]          |  <- 頂部工具列
+-------------------------+--------------------------------------------------+-----------------------+
| 🛠 控制項工具箱 (Toolbox)| 🎨 設計畫布表面 (Design Surface - 800 x 520)      | ⚙ 屬性與事件檢查器    |
| [Grid]      [StackPanel]| +----------------------------------------------+ | [屬性分頁] [事件分頁] |
| [Border]    [Canvas]    | |  [主 Grid 佈局面板]                           | | ▾ Common              |
| [Button]    [TextBox]   | |  🏷 系統帳戶設定                              | |   Name: saveBtn       |
| [CheckBox]  [Label]     | |  🔤 administrator                             | |   Type: Button        |
| [ComboBox]  [DataGrid]  | |  ☑ 啟用雙重驗證                               | | ▾ Layout              |
|                         | |  [儲存設定] ★ (8向控制框+磁吸導引線)           | |   Width: 130          |
| 🌳 元件層級樹 (Tree)    | +----------------------------------------------+ |   Height: 34          |
| ▾ 📄 AccountSettingsForm| ⚡ 即時代碼預覽 (Live Code Preview - Fluent DSL)  |   Margin: 8,8,8,8     |
|   ▾ 🔲 mainGrid         | auto saveBtn = Button("儲存設定 (Save)")         | ▾ Appearance          |
|      🏷 titleLabel      |     .Width(130).Height(34)                       |   Background: Primary |
|      🔘 saveBtn  ★      |     .OnClick([]() { OnSaveSettingsClick(); });   | ▾ Events              |
|      🔘 cancelBtn       |                                                  |   Click: OnSaveClick  |
+-------------------------+--------------------------------------------------+-----------------------+
| 狀態: 就緒 | 當前選取: saveBtn (Button) | 位置: (170, 190) 尺寸: 130 x 34 | 導引線: 啟用 | 歷史: 0 步       |  <- 底部狀態列
+----------------------------------------------------------------------------------------------------+
```

### 1. 頂部工具列 (Top Toolbar)
* **↶ 復原 (Undo) / ↷ 重做 (Redo)**：完整歷史記錄堆疊調度，支援單一與多選複合動作復原。
* **🗑 刪除元件**：將選取節點從 AST 移除並記錄至 Undo 堆疊。
* **▶ 預覽執行 (Live Run)**：一鍵以獨立原生 `app::Form` 視窗啟動當前表單，不含設計期 Adorner，支援真實點擊、鍵盤焦點導航與 ComboBox 彈出測試。
* **🎨 切換主題**：在深色 (Dark) 與淺色 (Light) 主題間熱切換，即時檢視設計成果對主題色彩 Token 的適配性。
* **📦 匯出 CMake 專案**：將 AST 產出為包含 `CMakeLists.txt`、`.designer.hpp`、`.hpp`、`.cpp`、`main.cpp` 的完整可編譯專案。

### 2. 左側欄 (工具箱與元件層級樹)
* **控制項工具箱 (Toolbox)**：提供容器（Grid, StackPanel, Border, Canvas）與基礎元件（Button, TextBox, CheckBox, Label, ComboBox, DataGrid），點選即可向當前選取容器掛載新子節點。
* **元件階層樹 (Hierarchy View)**：即時呈現象徵拓撲結構的樹狀檢視，點擊節點同步更新畫布選取與右側檢查器。

### 3. 中央主工作區 (畫布與即時代碼預覽)
* **設計表面 (`DesignSurface`)**：
  * **網格背景**：提供視覺間距參照。
  * **選取框與 8 向縮放控制錨點 (Handles)**：選取元件時外圍顯示高對比霓虹框與 8 個八向錨點（NW, N, NE, E, SE, S, SW, W），拖曳錨點即可即時改變尺寸。
  * **智慧對齊導引線 (Smart Snap Guides)**：拖曳移動或縮放元件時，若邊界或中心點接近父容器或其他元件，自動彈出洋紅色對齊導引線並自動磁吸。
* **即時代碼預覽 (`LiveCodePreviewPanel`)**：
  * 支援 5 種即時檢視分頁：`FluentDsl`、`ClassicDesignerHpp`、`ClassicHeader`、`ClassicSource`、`CMakeLists`。
  * 訂閱 AST 異動事件，在背景重新生成代碼。
  * 透過 `SourceMap` 雙向行號映射，在畫布點選元件時，代碼預覽自動滾動並高亮該元件對應之程式碼區塊。

### 4. 右側欄 (屬性與事件檢查器)
* **屬性分頁 (Properties Tab)**：
  * 自動解析類別繼承鏈（Visual $\to$ UIElement $\to$ Control $\to$ 衍生控制項），依外觀、佈局、文字、常見分類分組。
  * **多選交集編輯 (Multi-Selection Intersection)**：同時複選多個元件時，僅顯示共同屬性；值不同時呈現 `<Multiple Values>` 佔位符，修改後以單一複合 Undo 宏命令同步更新。
  * **主題 Token 挑選**：色彩支援直接套用 `ColorTokens::Primary()` 等語意代幣，切換主題自動適配。
* **事件分頁 (Events Tab)**：
  * 列出控制項支援的路由事件（如 `Click`, `TextChanged`）與 MVVM `RelayCommand` 綁定點。
  * 自動生成標準事件處理常式名稱（如 `OnSaveSettingsClick`）。

---

## 三、 實戰步驟教學 (Step-by-Step Tutorial)

### 步驟 1：啟動獨立表單設計器
在建置目錄中直接執行二進位檔：
```powershell
# Windows
.\build\bin\Debug\cppui-designer.exe

# Linux
./build/bin/Release/cppui-designer
```

### 步驟 2：從工具箱新增佈局面板與元件
1. 在左側「工具箱」點選 **🔲 Grid** 作為根容器。
2. 點選 **🏷 Label** 建立標題，並輸入「系統使用者設定」。
3. 點選 **🔤 TextBox** 建立帳號輸入框，並於檢查器設定 `Name: usernameInput`。
4. 點選 **🔘 Button** 建立按鈕，設定文字為「儲存設定 (Save)」。

### 步驟 3：精準幾何對齊與八向控制點操作
1. 點擊畫布上的按鈕，邊界將出現 **8 向縮放方塊**。
2. 拖曳右下角錨點（SE Handle）調整按鈕寬高至 `130 x 34`。
3. 拖曳按鈕本體，當按鈕左側與上方 TextBox 左側對齊時，畫布將自動繪製一條**洋紅色垂直導引線**並提供磁吸吸附。

### 步驟 4：設定主題語意代幣與外觀
1. 在右側檢查器外觀 (Appearance) 分類中，找到 `Background` 欄位。
2. 點擊色彩下拉選單，選取主題代幣 `ColorTokens::Primary()`。
3. 在工具列點擊 **🎨 切換主題**，觀察按鈕在深色與淺色主題下的動態色彩自適應。

### 步驟 5：綁定事件處理常式
1. 切換至右側檢查器的 **事件 (Events)** 分頁。
2. 在 `Click` 事件欄位中，預設已自動生成 `OnSaveSettingsClick`。
3. 按下 Enter，該事件綁定即刻同步寫入 AST。

### 步驟 6：一鍵「預覽執行 (Live Run)」
1. 點擊頂部工具列的 **▶ 預覽執行 (Live Run)** 按鈕。
2. 設計器將動態實例化一個獨立真實的 `app::Form` 視窗：
   - 移除了所有設計期 Adorner 與事件攔截盾。
   - 使用者可真實鍵入文字、點選 CheckBox、體驗按鈕的滑鼠懸停 (Hover) 與按壓 (Pressed) 視覺反饋。
   - 支援 Tab 鍵焦點順序（FocusManager）巡訪。

![Live Run Preview](../../images/designer_live_run_preview.png)
*圖 2：脫離設計期裝飾層的獨立真實試運行表單*

### 步驟 7：匯出完整 CMake 專案
1. 點擊工具列的 **📦 匯出 CMake 專案**。
2. 設計器自動於目標目錄產出標準檔案結構：
   ```text
   output/
   ├── CMakeLists.txt
   ├── .gitignore
   ├── include/
   │   └── EnterpriseApp/
   │       ├── AccountSettingsForm.designer.hpp  (元件宣告與佈局全量覆寫)
   │       └── AccountSettingsForm.hpp           (類別宣告)
   └── src/
       ├── AccountSettingsForm.cpp               (使用者邏輯，增量安全保護)
       └── main.cpp                              (進入點)
   ```
3. **增量安全保護驗證**：若使用者在 `AccountSettingsForm.cpp` 內編寫了自訂商業邏輯或全域變數，再次匯出時既有代碼完整保留，設計器僅在檔案末端追加新綁定的事件處理函式存根（Stubs）。

---

## 四、 快捷鍵一覽表 (Shortcuts Table)

| 快捷鍵 | 動作說明 | 對應功能 |
| :--- | :--- | :--- |
| **Ctrl + Z** | 復原 (Undo) | 復原上一次畫布移動、縮放或屬性修改 |
| **Ctrl + Y** | 重做 (Redo) | 重做已復原之編輯動作 |
| **Delete / Backspace** | 刪除元件 | 刪除畫布上選取的元件節點 |
| **Ctrl + 點擊** | 複選 (Multi-select) | 同時複選多個元件進行交集屬性編輯 |
| **F5** | 預覽執行 (Live Run) | 啟動真實試運行表單視窗 |
| **Ctrl + S** | 儲存專案 | 儲存 AST 為無損 `.cppui.json` |
| **Ctrl + E** | 專案匯出 | 匯出為完整 CMake 專案結構 |

---

## 五、 常見問答 (FAQ)

**Q: 為什麼表單設計器不需要依賴外部 XML/XAML 編譯器？**  
A: CPPUI 採用內建的 C++ 領域 AST 模型與代碼發射管線（`FormCodegenPipeline`），能直接發射出符合標準 C++ 語法的宣告式 Fluent DSL 或 partial class 代碼，免除任何第三方中繼編譯器（如 Qt MOC 或 XAML Compiler），建置速度極快。

**Q: 在設計畫布中，如何還原被修改的屬性至預設值？**  
A: 在右側屬性檢查器中，點擊任何屬性欄位右側的「重設 (Reset)」圖示，檢查器將自動查詢 `PropertyMetadataRegistry` 還原預設值，並推入一筆 `ChangePropertyCommand` 至 Undo 堆疊。

**Q: 匯出後的專案如何進行二次編譯？**  
A: 匯出的專案為標準現代 CMake 專案，只需進入匯出資料夾執行標準建置指令即可：
```bash
cmake -B build
cmake --build build --config Release
```
