/*
 * File: AsyncEventForm.cpp
 * Purpose: Implements AsyncEventForm layout and multi-threaded background task dispatching logic.
 */

#include "AsyncEventForm.hpp"
#include "ui/app/Application.hpp"
#include "ui/dsl/Fluent.hpp"
#include "compat/Compat.hpp"
#include <chrono>
#include <sstream>

namespace async_events_example {

AsyncEventForm::AsyncEventForm() {
    using namespace cppui::dsl;
    using cppui::core::Color;

    SetTitle("CPPUI - Asynchronous Events & Dispatcher Demo");
    SetFormSize(860, 580);

    SetContent(
        Border(
            VStack({
                // 1. Header 標題區
                Border(
                    VStack({
                        Label("CPPUI 非同步事件調度範例 (Async Events & Dispatcher)")
                            .FontSize(20.0f)
                            .FontWeight(cppui::text::FontWeight::Bold)
                            .Foreground(Color(255, 255, 255, 255)),
                        Label("展示背景 Worker 執行緒如何透過 Application::Post 安全投遞任務，實現無鎖 UI 狀態同步")
                            .FontSize(13.0f)
                            .Foreground(Color(150, 185, 230, 255))
                            .Margin(0.0f, 6.0f, 0.0f, 0.0f)
                    })
                )
                .Background(Color(22, 26, 36, 255))
                .BorderBrush(Color(45, 54, 75, 255))
                .BorderThickness(0.0f, 0.0f, 0.0f, 1.0f)
                .Padding(24.0f, 16.0f),

                // 2. 主卡片工作區
                VStack({
                    // 進度與狀態卡片
                    Border(
                        VStack({
                            Label("任務執行進度 (Task Progress)")
                                .FontSize(15.0f)
                                .FontWeight(cppui::text::FontWeight::SemiBold)
                                .Foreground(Color(230, 240, 255, 255)),

                            // 進度文字顯示
                            Label("目前進度: 0%")
                                .FontSize(26.0f)
                                .FontWeight(cppui::text::FontWeight::Bold)
                                .Foreground(Color(70, 215, 130, 255))
                                .Margin(0.0f, 4.0f, 0.0f, 6.0f)
                                .BindText(m_progress, [](int32_t val) {
                                    return "目前進度: " + compat::to_string(val) + "%";
                                }),

                            // 狀態說明
                            Label("狀態訊息")
                                .FontSize(14.0f)
                                .Foreground(Color(255, 205, 90, 255))
                                .BindText(m_status, [](const std::string& s) {
                                    return "狀態: " + s;
                                }),

                            // 執行緒資訊
                            Label("執行緒狀態")
                                .FontSize(12.0f)
                                .FontFamily("Consolas")
                                .Foreground(Color(140, 160, 190, 255))
                                .Margin(0.0f, 4.0f, 0.0f, 0.0f)
                                .BindText(m_threadInfo, [](const std::string& info) {
                                    return info;
                                })
                        }).Spacing(6.0f)
                    )
                    .Background(Color(19, 23, 32, 255))
                    .BorderBrush(Color(38, 46, 64, 255))
                    .BorderThickness(1.0f)
                    .CornerRadius(8.0f)
                    .Padding(20.0f),

                    // 事件日誌卡片
                    Border(
                        VStack({
                            Label("非同步事件接收日誌 (Dispatcher Event Stream)")
                                .FontSize(14.0f)
                                .FontWeight(cppui::text::FontWeight::SemiBold)
                                .Foreground(Color(210, 225, 245, 255)),

                            Label("")
                                .FontSize(12.0f)
                                .FontFamily("Consolas")
                                .Foreground(Color(120, 200, 255, 255))
                                .BindText(m_logOutput, [](const std::string& log) {
                                    return log;
                                })
                        }).Spacing(8.0f)
                    )
                    .Background(Color(16, 19, 27, 255))
                    .BorderBrush(Color(35, 42, 58, 255))
                    .BorderThickness(1.0f)
                    .CornerRadius(8.0f)
                    .Padding(16.0f),

                    // 操作按鈕列
                    HStack({
                        Button("啟動背景非同步任務")
                            .Width(200.0f)
                            .Height(38.0f)
                            .Background(Color(45, 120, 220, 255))
                            .Foreground(Color(255, 255, 255, 255))
                            .CornerRadius(6.0f)
                            .OnClick([this]() {
                                StartBackgroundTask();
                            }),

                        Button("停止 / 取消任務")
                            .Width(160.0f)
                            .Height(38.0f)
                            .Background(Color(190, 55, 65, 255))
                            .Foreground(Color(255, 255, 255, 255))
                            .CornerRadius(6.0f)
                            .OnClick([this]() {
                                StopBackgroundTask();
                            }),

                        Button("發送單次背景事件")
                            .Width(180.0f)
                            .Height(38.0f)
                            .Background(Color(55, 145, 100, 255))
                            .Foreground(Color(255, 255, 255, 255))
                            .CornerRadius(6.0f)
                            .OnClick([this]() {
                                FireOneShotEvent();
                            })
                    }).Spacing(12.0f)
                })
                .Padding(24.0f)
                .Spacing(16.0f)
            })
        )
        .Background(Color(13, 16, 22, 255))
        .Get()
    );
}

AsyncEventForm::~AsyncEventForm() {
    StopBackgroundTask();
}

void AsyncEventForm::StartBackgroundTask() {
    if (m_workerRunning.load()) {
        m_status = "背景任務已在執行中，請勿重複啟動！";
        return;
    }

    StopBackgroundTask(); // 確保舊執行緒已清理

    m_cancelRequested = false;
    m_workerRunning   = true;
    m_progress        = 0;
    m_status          = "背景工作進行中...";
    m_logOutput       = "[事件] 已從 UI 主執行緒派生背景 Worker 執行緒。";

    auto token = GetLifetimeToken();
    m_workerThread = std::make_unique<std::thread>([this, token]() {
        std::stringstream ss;
        ss << std::this_thread::get_id();
        std::string workerTid = ss.str();

        // 背景執行緒呼叫 CheckAccess 必然回傳 false
        bool isUIThread = cppui::app::Application::CheckAccess();
        compat::println("[Worker Thread {}] 任務啟動，CheckAccess() = {}", workerTid, isUIThread);

        for (int32_t step = 1; step <= 100; ++step) {
            if (m_cancelRequested.load()) {
                break;
            }

            // 模擬工作負荷
            std::this_thread::sleep_for(std::chrono::milliseconds(30));

            // 透過 Application::Post 封送回 UI 執行緒 (受 LifetimeToken 保護)
            cppui::app::Application::Post(token, [this, step, workerTid]() {
                m_progress = step;
                m_status   = "背景計算進度中... (" + compat::to_string(step) + "%)";
                m_threadInfo = "Worker TID: " + workerTid + " | UI 調度親和性: PASS (已於主執行緒執行)";
                if (step % 25 == 0 || step == 100) {
                    m_logOutput = "[Post 回呼] 主執行緒已接收並更新進度: " + compat::to_string(step) + "%";
                }
            });
        }

        const bool wasCancelled = m_cancelRequested.load();
        cppui::app::Application::Post(token, [this, wasCancelled]() {
            m_workerRunning = false;
            if (wasCancelled) {
                m_status    = "任務已由使用者手動取消。";
                m_logOutput = "[事件] 背景任務已中斷退出。";
            } else {
                m_status    = "任務順利完成！進度達 100%。";
                m_logOutput = "[事件] 背景任務已圓滿結束。";
            }
        });
    });
}

void AsyncEventForm::StopBackgroundTask() {
    if (m_workerRunning.load()) {
        m_cancelRequested = true;
    }
    if (m_workerThread && m_workerThread->joinable()) {
        m_workerThread->join();
    }
    m_workerThread.reset();
    m_workerRunning = false;
}

void AsyncEventForm::FireOneShotEvent() {
    m_eventCounter++;
    const uint32_t currentEventId = m_eventCounter;
    auto token = GetLifetimeToken();

    // 啟動暫時性背景執行緒投遞單次事件 (受 LifetimeToken 保護，視窗關閉時任務自動略過，杜絕 UAF)
    std::thread([this, token, currentEventId]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        cppui::app::Application::Post(token, [this, currentEventId]() {
            m_status    = "已接收單次背景事件 #" + compat::to_string(currentEventId);
            m_logOutput = "[OneShot] 單次背景事件 #" + compat::to_string(currentEventId) + " 已由主執行緒消費處理！";
        });
    }).detach();
}

void AsyncEventForm::OnFormClosing(cppui::app::FormClosingEventArgs& e) {
    Form::OnFormClosing(e);
    StopBackgroundTask();
    compat::println("[Lifecycle] AsyncEventForm 視窗關閉，所有非同步工作已清理完畢。");
}

} // namespace async_events_example
