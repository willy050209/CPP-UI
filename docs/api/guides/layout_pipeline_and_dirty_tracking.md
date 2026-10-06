# 雙階段排版管線與髒標記追蹤指南 (Layout Pipeline & Dirty Tracking)

## 1. 雙階段排版管線架構

CPPUI 採用類似 Avalonia / WPF 的雙階段微架構管線：

```
       [LayoutManager::UpdateLayout]
                     |
       +-------------+-------------+
       |                           |
       v                           v
  [1. Measure 階段]          [2. Arrange 階段]
AvailableSize 由上而下傳遞     FinalRect 由上而下傳遞
  DesiredSize 由下而上計算     RenderBounds/VisualOffset 確定
```

### 階段一：Measure (測量)
- 父容器向子元件傳入可用空間約束（`availableSize`）。
- `UIElement::Measure` 扣除 `Margin` 後呼叫子類別 `MeasureOverride`。
- 子元件計算內容所需空間，結合 `MinWidth`、`MaxWidth` 與 `Margin`，快取為 `DesiredSize`。
- 若可用約束未變且 `m_isMeasureValid == true`，直接命中快取，耗時趨近於 0 ms。

### 階段二：Arrange (排列)
- 父容器依據排列規則為子元件配置目標槽位矩形（`finalRect`）。
- `UIElement::Arrange` 根據 `HorizontalAlignment` 與 `VerticalAlignment` 計算目標位移與尺寸。
- 配置完成後更新 `RenderBounds` 與 `VisualOffset`，最後呼叫 `ArrangeOverride` 讓容器遞迴排列子元件。

---

## 2. 精確髒標記追蹤與隔離性 (Dirty Tracking Isolation)

為達成高訊噪比之極致效能，CPPUI 實作嚴格隔離的三級標記系統：

| 標記方法 | 標記屬性 | 影響層級 | 向上冒泡 | 典型情境 |
|---|---|---|---|---|
| `InvalidateMeasure()` | `m_isMeasureValid = false`, `m_isArrangeValid = false` | 排版幾何重新計算 | **是**（沿視覺樹向上冒泡至根節點） | 文字內容變更、字型尺寸調整、容器增刪子節點、Margin 調整 |
| `InvalidateArrange()` | `m_isArrangeValid = false` | 排列位置重新配置 | **是**（通知父節點調整插槽） | 對齊變更（Left -> Center）、Canvas SetLeft/Top |
| `InvalidateVisual()` | `m_isVisualValid = false` | 僅重新錄製繪製流 | **否**（完全隔離於 Measure/Arrange） | 文字前景色彩變更、背景色調整、筆刷更動 |

> **隔離性保證**：
> 變更色彩、透明度或筆刷時，排版樹的 Measure 與 Arrange 快取維持 **100% 命中**，絕不重跑任何度量與幾何計算！
