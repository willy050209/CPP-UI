/**
 * @file Visual.hpp
 * @brief 視覺樹最輕量節點基底類別，負責親緣階層維護、座標位移、透明度、命中測試與遞迴渲染
 */

#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include "compat/Compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/rendering/DrawingContext.hpp"

namespace cppui {
namespace core {

/// <summary>
/// 視覺樹最輕量節點基底類別 (Visual)
/// 負責親緣階層維護、局部座標位移 (VisualOffset)、透明度、裁剪與遞迴渲染通道。
/// </summary>
class Visual : public std::enable_shared_from_this<Visual> {
public:
    /// <summary>
    /// 預設建構子。
    /// </summary>
    Visual() = default;

    /// <summary>
    /// 虛擬解構子。
    /// </summary>
    virtual ~Visual() = default;

    // 禁止拷貝，支援移動
    Visual(const Visual&) = delete;
    Visual& operator=(const Visual&) = delete;
    Visual(Visual&&) noexcept = default;
    Visual& operator=(Visual&&) noexcept = default;

    // --- 親緣關係 ---

    /// <summary>
    /// 取得視覺樹父節點指標。
    /// </summary>
    /// <returns>父節點指標，若無則為 nullptr。</returns>
    COMPAT_NODISCARD Visual* GetVisualParent() const noexcept { return m_visualParent; }

    /// <summary>
    /// 取得視覺子節點數量。
    /// </summary>
    /// <returns>子節點數量。</returns>
    COMPAT_NODISCARD size_t GetVisualChildrenCount() const noexcept { return m_visualChildren.size(); }

    /// <summary>
    /// 依索引取得視覺子節點指標。
    /// </summary>
    /// <param name="index">子節點索引。</param>
    /// <returns>子節點指標，若越界回傳 nullptr。</returns>
    COMPAT_NODISCARD Visual* GetVisualChild(size_t index) const;

    /// <summary>
    /// 加入視覺子節點。
    /// </summary>
    /// <param name="child">子節點智慧指標，不可為空。</param>
    void AddVisualChild(std::shared_ptr<Visual> child);

    /// <summary>
    /// 移除指定之視覺子節點。
    /// </summary>
    /// <param name="child">欲移除之子節點智慧指標。</param>
    void RemoveVisualChild(const std::shared_ptr<Visual>& child);

    /// <summary>
    /// 依原始指標移除指定之視覺子節點 (解構安全)。
    /// </summary>
    /// <param name="child">欲移除之子節點裸指標。</param>
    void RemoveVisualChild(Visual* child);

    /// <summary>
    /// 清除所有視覺子節點。
    /// </summary>
    void ClearVisualChildren();

    /// <summary>
    /// 將指定之子視覺節點移至清單最末端 (最上層/前景)。
    /// </summary>
    /// <param name="child">子節點原始指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">若 child 為 nullptr 則擲出。</exception>
    void BringVisualChildToFront(Visual* child);

    /// <summary>
    /// 將指定之子視覺節點移至清單開頭 (最底層/背景)。
    /// </summary>
    /// <param name="child">子節點原始指標，不可為 nullptr。</param>
    /// <exception cref="std::invalid_argument">若 child 為 nullptr 則擲出。</exception>
    void SendVisualChildToBack(Visual* child);

    // --- 幾何與變換屬性 ---

    /// <summary>
    /// 取得局部座標位移 (VisualOffset)。
    /// </summary>
    /// <returns>座標位移向量。</returns>
    COMPAT_NODISCARD Point GetVisualOffset() const noexcept { return m_visualOffset; }

    /// <summary>
    /// 設定局部座標位移。
    /// </summary>
    /// <param name="offset">座標位移向量。</param>
    void SetVisualOffset(Point offset) noexcept { m_visualOffset = offset; }

    /// <summary>
    /// 取得節點視覺邊界矩形。
    /// </summary>
    /// <returns>邊界矩形。</returns>
    COMPAT_NODISCARD Rect GetVisualBounds() const noexcept { return m_visualBounds; }

    /// <summary>
    /// 設定節點視覺邊界矩形。
    /// </summary>
    /// <param name="bounds">邊界矩形。</param>
    void SetVisualBounds(const Rect& bounds) noexcept { m_visualBounds = bounds; }

    /// <summary>
    /// 取得節點不透明度 (0.0f - 1.0f)。
    /// </summary>
    /// <returns>不透明度值。</returns>
    COMPAT_NODISCARD float GetOpacity() const noexcept { return m_opacity; }

    /// <summary>
    /// 設定節點不透明度 (內部將自動限制於 [0.0, 1.0])。
    /// </summary>
    /// <param name="opacity">不透明度值。</param>
    void SetOpacity(float opacity) noexcept { m_opacity = compat::clamp(opacity, 0.0f, 1.0f); }

    /// <summary>
    /// 檢查節點是否可見。
    /// </summary>
    /// <returns>若可見回傳 true，隱藏回傳 false。</returns>
    COMPAT_NODISCARD bool IsVisible() const noexcept { return m_isVisible; }

    /// <summary>
    /// 設定節點是否可見。
    /// </summary>
    /// <param name="visible">是否可見。</param>
    void SetVisible(bool visible) noexcept { m_isVisible = visible; }

    /// <summary>
    /// 取得是否裁剪至邊界矩形內。
    /// </summary>
    /// <returns>若啟用邊界裁剪傳回 true。</returns>
    COMPAT_NODISCARD bool GetClipToBounds() const noexcept { return m_clipToBounds; }

    /// <summary>
    /// 設定是否裁剪至邊界矩形內。
    /// </summary>
    /// <param name="clip">是否啟用邊界裁剪。</param>
    void SetClipToBounds(bool clip) noexcept { m_clipToBounds = clip; }

    // --- 座標系變換 ---

    /// <summary>
    /// 將螢幕/全域座標轉換為本節點之局部使用者座標。
    /// </summary>
    /// <param name="screenPoint">全域座標點。</param>
    /// <returns>局部座標點。</returns>
    COMPAT_NODISCARD Point PointToClient(Point screenPoint) const;

    /// <summary>
    /// 將本節點局部座標轉換為螢幕/全域座標。
    /// </summary>
    /// <param name="clientPoint">局部座標點。</param>
    /// <returns>全域座標點。</returns>
    COMPAT_NODISCARD Point PointToScreen(Point clientPoint) const;

    /// <summary>
    /// 將座標由本節點轉換至指定之祖先節點座標系。
    /// </summary>
    /// <param name="p">待轉換座標點。</param>
    /// <param name="ancestor">目標祖先節點指標。</param>
    /// <returns>祖先座標系點。</returns>
    COMPAT_NODISCARD Point TransformToAncestor(Point p, const Visual* ancestor) const;

    /// <summary>
    /// 將指定祖先節點座標系下的座標轉換至本節點局部座標系。
    /// </summary>
    /// <param name="p">祖先座標系點。</param>
    /// <param name="ancestor">目標祖先節點指標。</param>
    /// <returns>局部座標系點。</returns>
    COMPAT_NODISCARD Point TransformFromAncestor(Point p, const Visual* ancestor) const;

    // --- 命中測試 ---

    /// <summary>
    /// 依據局部座標執行階層命中測試，由下而上探查最深層之視覺節點。
    /// </summary>
    /// <param name="localPoint">局部命中點。</param>
    /// <returns>命中的 Visual 節點指標，未命中傳回 nullptr。</returns>
    COMPAT_NODISCARD virtual Visual* HitTest(Point localPoint);

    // --- 渲染管線 ---

    /// <summary>
    /// 執行遞迴渲染管線通道，套用透明度、裁切並繪製子節點。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    void Render(rendering::DrawingContext& dc);

    /// <summary>
    /// 虛擬自訂繪製掛鉤，供衍生節點覆寫自訂外觀向量繪製。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    virtual void OnRender(rendering::DrawingContext& dc) { (void)dc; }

    /// <summary>
    /// 虛擬後置繪製掛鉤，在所有子視覺節點繪製完成後呼叫（適用於前景裝飾、浮動卷軸等）。
    /// </summary>
    /// <param name="dc">繪圖上下文。</param>
    virtual void OnPostRender(rendering::DrawingContext& dc) { (void)dc; }

protected:
    /// <summary>
    /// 設定視覺父節點內部指標。
    /// </summary>
    /// <param name="parent">父節點指標。</param>
    void SetVisualParent(Visual* parent) noexcept { m_visualParent = parent; }

    Visual*                                m_visualParent = nullptr;
    std::vector<std::shared_ptr<Visual>>   m_visualChildren;
    Point                                  m_visualOffset{ 0.0f, 0.0f };
    Rect                                   m_visualBounds{ 0.0f, 0.0f, 0.0f, 0.0f };
    float                                  m_opacity = 1.0f;
    bool                                   m_isVisible = true;
    bool                                   m_clipToBounds = false;
};

} // namespace core
} // namespace cppui
