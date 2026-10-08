/*
 * File: AsyncEventForm.hpp
 * Purpose: Declares AsyncEventForm demonstrating cross-thread asynchronous event dispatching
 *          via Application::Post and Dispatcher.
 */

#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include "ui/app/Form.hpp"
#include "ui/reactive/Signal.hpp"

namespace async_events_example {

/// <summary>
/// 非同步事件與執行緒調度示範視窗表單
/// </summary>
class AsyncEventForm : public cppui::app::Form {
public:
    /// <summary>
    /// 建構 AsyncEventForm 並初始化介面佈局。
    /// </summary>
    AsyncEventForm();

    /// <summary>
    /// 解構 AsyncEventForm 實例，確保工作執行緒安全停止。
    /// </summary>
    ~AsyncEventForm() override;

    /// <summary>
    /// 啟動模擬背景耗時計算與資料傳輸任務。
    /// </summary>
    void StartBackgroundTask();

    /// <summary>
    /// 請求取消或停止目前正在運行的背景任務。
    /// </summary>
    void StopBackgroundTask();

    /// <summary>
    /// 從背景執行緒投遞單次即時非同步事件至 UI 執行緒。
    /// </summary>
    void FireOneShotEvent();

protected:
    /// <summary>
    /// 視窗即將關閉生命週期回呼，負責安全終止 Worker 執行緒。
    /// </summary>
    /// <param name="e">視窗關閉事件參數。</param>
    void OnFormClosing(cppui::app::FormClosingEventArgs& e) override;

private:
    std::atomic<bool>              m_workerRunning{false};
    std::atomic<bool>              m_cancelRequested{false};
    std::unique_ptr<std::thread>   m_workerThread;
    uint32_t                       m_eventCounter{0};

    // 響應式狀態信號 (由主執行緒消費更新，驅動 UI 即時反應)
    cppui::reactive::Signal<int32_t>     m_progress{0};
    cppui::reactive::Signal<std::string> m_status{"準備就緒，請點擊下方按鈕啟動背景任務。"};
    cppui::reactive::Signal<std::string> m_threadInfo{"執行緒狀態：閒置"};
    cppui::reactive::Signal<std::string> m_logOutput{"[系統] 應用程式已初始化，等待非同步事件觸發。"};
};

} // namespace async_events_example
