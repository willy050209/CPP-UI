/**
 * @file Timer.hpp
 * @brief 計時器元件，在使用者定義的間隔內觸發事件 (對標 System.Windows.Forms.Timer)
 */

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include "compat/compat.hpp"

namespace cppui {
namespace app {

/// <summary>
/// 在使用者自訂間隔內於 UI 執行緒週期性觸發 Tick 事件之計時器元件 (對標 System.Windows.Forms.Timer)。
/// </summary>
class Timer {
public:
    using TickHandler = std::function<void()>;
    using TickWithSenderHandler = std::function<void(Timer&)>;

    /// <summary>
    /// 建構預設計時器 (預設間隔 100 毫秒，未啟用)。
    /// </summary>
    Timer();

    /// <summary>
    /// 建構具備指定間隔之計時器 (未啟用)。
    /// </summary>
    /// <param name="intervalMs">計時觸發間隔毫秒數，必須大於 0。</param>
    /// <exception cref="std::invalid_argument">當 intervalMs 小於或等於 0 時擲出。</exception>
    explicit Timer(int32_t intervalMs);

    /// <summary>
    /// 解構子，自動停止計時器並釋放排程資源。
    /// </summary>
    ~Timer();

    // 禁止拷貝
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    // 支援移動語意
    Timer(Timer&& other) noexcept;
    Timer& operator=(Timer&& other) noexcept;

    /// <summary>
    /// 取得計時期間隔時間 (毫秒)。
    /// </summary>
    /// <returns>間隔毫秒數。</returns>
    COMPAT_NODISCARD int32_t GetInterval() const noexcept;

    /// <summary>
    /// 設定計時期間隔時間 (毫秒)。
    /// </summary>
    /// <param name="intervalMs">間隔毫秒數，必須大於 0。</param>
    /// <exception cref="std::invalid_argument">當 intervalMs 小於或等於 0 時擲出。</exception>
    void SetInterval(int32_t intervalMs);

    /// <summary>
    /// 檢查計時器是否處於啟用執行狀態。
    /// </summary>
    /// <returns>若已啟用傳回 true，否則傳回 false。</returns>
    COMPAT_NODISCARD bool IsEnabled() const noexcept;

    /// <summary>
    /// 設定計時器之啟用狀態。
    /// 設定為 true 等同於呼叫 Start()；設定為 false 等同於呼叫 Stop()。
    /// </summary>
    /// <param name="enabled">true 啟用計時器，false 停用計時器。</param>
    void SetEnabled(bool enabled);

    /// <summary>
    /// 啟動計時器 (對標 WinForms Timer.Start())。
    /// </summary>
    void Start();

    /// <summary>
    /// 停止計時器 (對標 WinForms Timer.Stop())。
    /// </summary>
    void Stop();

    /// <summary>
    /// 重新啟動計時器 (重設間隔倒數並開始計時)。
    /// </summary>
    void Restart();

    /// <summary>
    /// 立即以程式碼強制觸發一次 Tick 事件 (可用於單元測試或強制刷新)。
    /// </summary>
    void PerformTick();

    /// <summary>
    /// 取得自訂標籤指標或使用者資料 (Tag)。
    /// </summary>
    /// <returns>使用者自訂資料指標。</returns>
    COMPAT_NODISCARD void* GetTag() const noexcept;

    /// <summary>
    /// 設定自訂標籤指標或使用者資料 (Tag)。
    /// </summary>
    /// <param name="tag">使用者自訂資料指標。</param>
    void SetTag(void* tag) noexcept;

    /// <summary>
    /// 註冊無參數之 Tick 事件處理常式。
    /// </summary>
    /// <param name="handler">回呼常式，不可為空。</param>
    /// <exception cref="std::invalid_argument">當 handler 為空時擲出。</exception>
    void OnTick(TickHandler handler);

    /// <summary>
    /// 註冊接收 Timer 引用之 Tick 事件處理常式。
    /// </summary>
    /// <param name="handler">回呼常式，不可為空。</param>
    /// <exception cref="std::invalid_argument">當 handler 為空時擲出。</exception>
    void OnTick(TickWithSenderHandler handler);

    /// <summary>
    /// 註冊無參數之 Tick 事件處理常式 (對標 WinForms timer.Tick += handler)。
    /// </summary>
    /// <param name="handler">回呼常式，不可為空。</param>
    /// <exception cref="std::invalid_argument">當 handler 為空時擲出。</exception>
    void AddTickHandler(TickHandler handler);

    /// <summary>
    /// 註冊接收 Timer 引用之 Tick 事件處理常式。
    /// </summary>
    /// <param name="handler">回呼常式，不可為空。</param>
    /// <exception cref="std::invalid_argument">當 handler 為空時擲出。</exception>
    void AddTickHandler(TickWithSenderHandler handler);

    /// <summary>
    /// 清除所有已註冊之 Tick 事件處理常式。
    /// </summary>
    void ClearTickHandlers() noexcept;

    /// <summary>
    /// 取得目前已註冊之 Tick 處理常式數量。
    /// </summary>
    /// <returns>處理常式總數。</returns>
    COMPAT_NODISCARD size_t GetTickHandlerCount() const noexcept;

    struct Impl;

private:
    std::shared_ptr<Impl> m_impl;
};

} // namespace app

namespace controls {
    using Timer = cppui::app::Timer;
} // namespace controls
} // namespace cppui
