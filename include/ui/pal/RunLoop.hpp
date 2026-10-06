#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <vector>
#include "compat/compat.hpp"
#include "ui/pal/IPlatformWindow.hpp"

namespace cppui {
namespace pal {

/// <summary>
/// 主執行緒低延遲事件循環 (RunLoop)
/// </summary>
class RunLoop {
public:
    RunLoop() = default;
    ~RunLoop() = default;

    // 禁止拷貝
    RunLoop(const RunLoop&) = delete;
    RunLoop& operator=(const RunLoop&) = delete;

    /// <summary>
    /// 綁定受控視窗
    /// </summary>
    void AttachWindow(std::shared_ptr<IPlatformWindow> window) {
        m_window = std::move(window);
    }

    /// <summary>
    /// 設定渲染回呼函式 (每次畫面刷新時觸發)
    /// </summary>
    void SetRenderCallback(std::function<void()> callback) {
        m_renderCallback = std::move(callback);
    }

    /// <summary>
    /// 請求下一幀重繪
    /// </summary>
    void RequestRedraw() noexcept {
        m_needsRedraw = true;
        if (m_window) {
            m_window->RequestRedraw();
        }
    }

    /// <summary>
    /// 終止事件循環
    /// </summary>
    void Stop() noexcept {
        m_running = false;
    }

    /// <summary>
    /// 查詢是否正在運行
    /// </summary>
    COMPAT_NODISCARD bool IsRunning() const noexcept {
        return m_running;
    }

    /// <summary>
    /// 啟動主執行緒事件循環
    /// </summary>
    void Run() {
        m_running = true;

        while (m_running) {
            if (m_window && m_window->IsClosed()) {
                m_running = false;
                break;
            }

            // 1. 處理系統原生訊息
            if (m_window) {
                m_window->PollEvents();
            }

            // 2. 若有重繪需求，執行渲染
            if (m_needsRedraw) {
                m_needsRedraw = false;
                if (m_renderCallback) {
                    m_renderCallback();
                }
            }

            // 3. 低延遲休眠避免 100% CPU 空轉 (若無即時繪製需求，進行短暫等待)
            if (!m_needsRedraw && m_window) {
                // 等待最多 16ms (對應約 60 FPS 的間隔)
                m_window->WaitEvents(16);
            }
        }
    }

private:
    std::shared_ptr<IPlatformWindow> m_window = nullptr;
    std::function<void()>            m_renderCallback;
    bool                             m_running = false;
    bool                             m_needsRedraw = true;
};

} // namespace pal
} // namespace cppui
