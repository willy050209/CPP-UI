/**
 * @file LinearArena.hpp
 * @brief 高效連續平坦記憶體池，提供指令的高速分配與零開銷幀末重設
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <new>
#include <algorithm>
#include "compat/compat.hpp"

namespace cppui {
namespace rendering {

/// <summary>
/// 高效連續平坦記憶體池 (Linear Arena Allocator)
/// 專為每幀產生的大量繪圖指令設計，具備 8 字節對齊、$O(1)$ 指標分配與零開銷幀末重設。
/// </summary>
class LinearArena {
public:
    static constexpr size_t DEFAULT_CAPACITY = 256 * 1024; // 預設 256 KB

    /// <summary>
    /// 建構平坦記憶體池。
    /// </summary>
    /// <param name="initialCapacity">初始容量位元組數。</param>
    explicit LinearArena(size_t initialCapacity = DEFAULT_CAPACITY) {
        m_capacity = std::max<size_t>(initialCapacity, 4096);
        m_buffer.resize(m_capacity);
        m_offset = 0;
    }

    /// <summary>預設解構子。</summary>
    ~LinearArena() = default;

    // 禁止拷貝，允許移動
    LinearArena(const LinearArena&) = delete;
    LinearArena& operator=(const LinearArena&) = delete;

    /// <summary>移動建構子。</summary>
    LinearArena(LinearArena&& other) noexcept
        : m_buffer(std::move(other.m_buffer)),
          m_capacity(other.m_capacity),
          m_offset(other.m_offset) {
        other.m_capacity = 0;
        other.m_offset = 0;
    }

    /// <summary>移動賦值運算子。</summary>
    LinearArena& operator=(LinearArena&& other) noexcept {
        if (this != &other) {
            m_buffer = std::move(other.m_buffer);
            m_capacity = other.m_capacity;
            m_offset = other.m_offset;
            other.m_capacity = 0;
            other.m_offset = 0;
        }
        return *this;
    }

    /// <summary>
    /// 在連續平坦記憶體池中分配指定字節數，保證 alignment 對齊。
    /// </summary>
    /// <param name="bytes">欲分配之位元組數。</param>
    /// <param name="alignment">對齊位元組邊界，預設為 8。</param>
    /// <returns>已分配之記憶體指標。</returns>
    void* Allocate(size_t bytes, size_t alignment = 8) {
        // 計算對齊後的起始位址
        size_t alignedOffset = (m_offset + (alignment - 1)) & ~(alignment - 1);
        size_t newOffset = alignedOffset + bytes;

        if (COMPAT_UNLIKELY(newOffset > m_capacity)) {
            // 自動按倍數擴展平坦緩衝區，並保留記憶體以利後續幀複用
            size_t newCapacity = std::max(m_capacity * 2, newOffset + DEFAULT_CAPACITY);
            m_buffer.resize(newCapacity);
            m_capacity = newCapacity;
        }

        uint8_t* ptr = m_buffer.data() + alignedOffset;
        m_offset = newOffset;
        return static_cast<void*>(ptr);
    }

    /// <summary>
    /// 型別化分配輔助函式。
    /// </summary>
    /// <typeparam name="T">目標型別。</typeparam>
    /// <param name="count">元素數量，預設為 1。</param>
    /// <returns>指向已分配型別 T 物件空間之指標。</returns>
    template <typename T>
    T* Allocate(size_t count = 1) {
        return static_cast<T*>(Allocate(sizeof(T) * count, alignof(T)));
    }

    /// <summary>
    /// 幀末 O(1) 重設指標，保留實體記憶體供下一幀重用。
    /// </summary>
    void Reset() noexcept {
        m_offset = 0;
    }

    /// <summary>
    /// 取得目前已使用字節數。
    /// </summary>
    /// <returns>已使用位元組數。</returns>
    size_t GetUsedBytes() const noexcept {
        return m_offset;
    }

    /// <summary>
    /// 取得目前總容量 (字節)。
    /// </summary>
    /// <returns>總容量位元組數。</returns>
    size_t GetCapacity() const noexcept {
        return m_capacity;
    }

    /// <summary>
    /// 取得平坦緩衝區原始資料指標 (常數重載)。
    /// </summary>
    /// <returns>平坦緩衝區唯讀指標。</returns>
    COMPAT_NODISCARD const uint8_t* GetData() const noexcept {
        return m_buffer.data();
    }

    /// <summary>
    /// 取得平坦緩衝區原始資料指標。
    /// </summary>
    /// <returns>平坦緩衝區指標。</returns>
    COMPAT_NODISCARD uint8_t* GetData() noexcept {
        return m_buffer.data();
    }

    /// <summary>
    /// 取得目前已使用平坦資料之連續切片視圖 (span, 唯讀)。
    /// </summary>
    /// <returns>唯讀 span 視圖。</returns>
    COMPAT_NODISCARD compat::span<const uint8_t> GetSpan() const noexcept {
        return compat::span<const uint8_t>(m_buffer.data(), m_offset);
    }

    /// <summary>
    /// 取得目前已使用平坦資料之連續切片視圖 (span, 可寫)。
    /// </summary>
    /// <returns>可寫 span 視圖。</returns>
    COMPAT_NODISCARD compat::span<uint8_t> GetSpan() noexcept {
        return compat::span<uint8_t>(m_buffer.data(), m_offset);
    }

private:
    std::vector<uint8_t> m_buffer;
    size_t               m_capacity = 0;
    size_t               m_offset = 0;
};

} // namespace rendering
} // namespace cppui
