#pragma once

#include <cstddef>
#include <cstdint>
#include "compat/compat.hpp"
#include "ui/rendering/LinearArena.hpp"
#include "ui/rendering/DrawOp.hpp"

namespace cppui::rendering {

/// <summary>
/// 輕量繪圖指令串流容器 (DisplayList)
/// 封裝連續平坦記憶體池，提供指令的高速二進位序列化與無鎖迭代重播。
/// </summary>
class DisplayList {
public:
    explicit DisplayList(size_t initialCapacity = LinearArena::DEFAULT_CAPACITY)
        : m_arena(initialCapacity) {}

    ~DisplayList() = default;

    // 禁止拷貝，允許移動
    DisplayList(const DisplayList&) = delete;
    DisplayList& operator=(const DisplayList&) = delete;

    DisplayList(DisplayList&& other) noexcept
        : m_arena(std::move(other.m_arena)),
          m_opCount(other.m_opCount) {
        other.m_opCount = 0;
    }

    DisplayList& operator=(DisplayList&& other) noexcept {
        if (this != &other) {
            m_arena = std::move(other.m_arena);
            m_opCount = other.m_opCount;
            other.m_opCount = 0;
        }
        return *this;
    }

    /// <summary>
    /// 將帶有 Payload 的繪圖操作寫入平坦記憶體池 (保證 8 字節對齊)
    /// </summary>
    template <typename PayloadType>
    void PushOp(DrawOpType type, const PayloadType& payload, uint16_t flags = 0) {
        size_t rawSize = sizeof(DrawOpHeader) + sizeof(PayloadType);
        size_t alignedSize = (rawSize + 7) & ~static_cast<size_t>(7);
        uint8_t* mem = static_cast<uint8_t*>(m_arena.Allocate(alignedSize, 8));

        auto* header = reinterpret_cast<DrawOpHeader*>(mem);
        header->type = type;
        header->flags = flags;
        header->size = static_cast<uint32_t>(alignedSize);

        auto* destPayload = reinterpret_cast<PayloadType*>(mem + sizeof(DrawOpHeader));
        *destPayload = payload;

        m_opCount++;
    }

    /// <summary>
    /// 寫入僅有標頭的操作 (如 PopClip, PopOpacity, Restore，保證 8 字節對齊)
    /// </summary>
    void PushHeaderOnly(DrawOpType type, uint16_t flags = 0) {
        size_t rawSize = sizeof(DrawOpHeader);
        size_t alignedSize = (rawSize + 7) & ~static_cast<size_t>(7);
        uint8_t* mem = static_cast<uint8_t*>(m_arena.Allocate(alignedSize, 8));

        auto* header = reinterpret_cast<DrawOpHeader*>(mem);
        header->type = type;
        header->flags = flags;
        header->size = static_cast<uint32_t>(alignedSize);

        m_opCount++;
    }

    /// <summary>
    /// 遍歷重播所有二進位繪圖操作
    /// Visitor 簽章: void(const DrawOpHeader& header, const void* payload)
    /// </summary>
    template <typename Visitor>
    void ForEach(Visitor&& visitor) const {
        const uint8_t* current = m_arena.GetData();
        const uint8_t* end = current + m_arena.GetUsedBytes();

        while (current < end) {
            const auto* header = reinterpret_cast<const DrawOpHeader*>(current);
            if (COMPAT_UNLIKELY(header->size == 0)) {
                break; // 防禦死循環
            }

            const void* payload = current + sizeof(DrawOpHeader);
            visitor(*header, payload);

            current += header->size;
        }
    }

    /// <summary>
    /// 幀末重設整個命令流 (O(1) 歸零，保留實體緩衝區)
    /// </summary>
    void Clear() noexcept {
        m_arena.Reset();
        m_opCount = 0;
    }

    size_t GetOpCount() const noexcept { return m_opCount; }
    size_t GetTotalBytes() const noexcept { return m_arena.GetUsedBytes(); }
    bool IsEmpty() const noexcept { return m_opCount == 0; }

private:
    LinearArena m_arena;
    size_t      m_opCount = 0;
};

} // namespace cppui::rendering
