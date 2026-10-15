/**
 * @file LinearArena.hpp
 * @brief 高效連續平坦記憶體池，提供指令的高速分配與零開銷幀末重設
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#include <new>
#include <algorithm>
#include <type_traits>
#include "compat/Compat.hpp"

namespace cppui {
namespace rendering {

/// <summary>
/// 連續平坦記憶體池專屬切片視圖 (ArenaSpan)
/// 強制約束目標型別 T 具備平凡解構特性，生命週期由 LinearArena 統一管理。
/// </summary>
/// <typeparam name="T">元素型別，必須為可平凡解構型別。</typeparam>
template <typename T>
class ArenaSpan {
public:
    static_assert(std::is_trivially_destructible<T>::value,
        "ArenaSpan only supports trivially destructible types!");

    constexpr ArenaSpan() noexcept : m_data(nullptr), m_size(0) {}
    constexpr ArenaSpan(T* data, size_t size) noexcept : m_data(data), m_size(size) {}

    COMPAT_NODISCARD constexpr T* data() const noexcept { return m_data; }
    COMPAT_NODISCARD constexpr size_t size() const noexcept { return m_size; }
    COMPAT_NODISCARD constexpr bool empty() const noexcept { return m_size == 0; }

    constexpr T& operator[](size_t index) const noexcept { return m_data[index]; }
    constexpr T* begin() const noexcept { return m_data; }
    constexpr T* end() const noexcept { return m_data + m_size; }

    COMPAT_NODISCARD compat::span<T> AsSpan() const noexcept {
        return compat::span<T>(m_data, m_size);
    }

private:
    T*     m_data = nullptr;
    size_t m_size = 0;
};

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
    /// 在連續平坦記憶體池中分配指定字節數，保證 alignment 絕對位址嚴格對齊。
    /// </summary>
    /// <param name="bytes">欲分配之位元組數。</param>
    /// <param name="alignment">對齊位元組邊界，預設為 8。</param>
    /// <returns>已分配之記憶體指標。</returns>
    void* Allocate(size_t bytes, size_t alignment = 8) {
        if (COMPAT_UNLIKELY(bytes == 0)) return nullptr;

        // 確保 alignment 為 2 的次方且至少為 alignof(void*)
        if (COMPAT_UNLIKELY((alignment & (alignment - 1)) != 0 || alignment == 0)) {
            alignment = alignof(std::max_align_t);
        }

        // 預留最壞情況對齊填充與所需空間
        size_t worstCaseRequired = m_offset + alignment + bytes;
        if (COMPAT_UNLIKELY(worstCaseRequired > m_capacity)) {
            size_t newCapacity = std::max(m_capacity * 2, worstCaseRequired + DEFAULT_CAPACITY);
            m_buffer.resize(newCapacity);
            m_capacity = newCapacity;
        }

        // 基於緩衝區實體指標位址計算嚴格對齊
        uintptr_t baseAddr = reinterpret_cast<uintptr_t>(m_buffer.data() + m_offset);
        uintptr_t alignedAddr = (baseAddr + (alignment - 1)) & ~(static_cast<uintptr_t>(alignment - 1));
        size_t padding = static_cast<size_t>(alignedAddr - baseAddr);

        size_t newOffset = m_offset + padding + bytes;
        m_offset = newOffset;

        return reinterpret_cast<void*>(alignedAddr);
    }

    /// <summary>
    /// 型別化分配輔助函式。強制約束 T 為可平凡解構之型別。
    /// </summary>
    /// <typeparam name="T">目標型別，必須為可平凡解構型別。</typeparam>
    /// <param name="count">元素數量，預設為 1。</param>
    /// <returns>指向已分配型別 T 物件空間之指標。</returns>
    template <typename T>
    T* Allocate(size_t count = 1) {
        static_assert(std::is_trivially_destructible<T>::value,
            "LinearArena::Allocate only supports trivially destructible types because Reset() does not call destructors!");
        return static_cast<T*>(Allocate(sizeof(T) * count, alignof(T)));
    }

    /// <summary>
    /// 在連續平坦記憶體池中配置型別化陣列，並傳回 ArenaSpan 視圖。
    /// </summary>
    /// <typeparam name="T">元素型別。</typeparam>
    /// <param name="count">元素數量。</param>
    /// <returns>ArenaSpan 視圖。</returns>
    template <typename T>
    ArenaSpan<T> AllocateSpan(size_t count) {
        static_assert(std::is_trivially_destructible<T>::value,
            "LinearArena::AllocateSpan only supports trivially destructible types!");
        if (count == 0) return ArenaSpan<T>();
        T* ptr = Allocate<T>(count);
        return ArenaSpan<T>(ptr, count);
    }

    /// <summary>
    /// 在平坦記憶體池中配置並深拷貝字串內容，傳回生命週期繫結於 Arena 的字串視圖。
    /// </summary>
    /// <param name="str">來源字串視圖。</param>
    /// <returns>繫結於 Arena 平坦緩衝區之 string_view。</returns>
    compat::string_view AllocateString(compat::string_view str) {
        if (str.empty()) return compat::string_view();
        char* mem = static_cast<char*>(Allocate(str.size() + 1, alignof(char)));
        std::memcpy(mem, str.data(), str.size());
        mem[str.size()] = '\0';
        return compat::string_view(mem, str.size());
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
