#pragma once

#include <vector>
#include <memory>
#include "compat/compat.hpp"

namespace cppui {
namespace text {

/// <summary>
/// 雙向文字總體段落流向
/// </summary>
enum class BiDiDirection : uint8_t {
    LeftToRight = 0,
    RightToLeft = 1,
    Mixed       = 2
};

/// <summary>
/// 雙向文字分析單一方向切片 (BiDi Run)
/// </summary>
struct BiDiRun {
    size_t  startIndex     = 0;     // UTF-8 字串中的起始字節/字元索引
    size_t  length         = 0;     // 長度
    uint8_t embeddingLevel = 0;     // Unicode 雙向嵌入等級 (偶數為 LTR，奇數為 RTL)
    bool    isRtl          = false; // 是否為由右至左書寫
};

/// <summary>
/// Unicode 雙向文字分析器 (BiDiAnalyzer)
/// </summary>
class BiDiAnalyzer {
public:
    BiDiAnalyzer();
    ~BiDiAnalyzer();

    // 禁止複製
    BiDiAnalyzer(const BiDiAnalyzer&) = delete;
    BiDiAnalyzer& operator=(const BiDiAnalyzer&) = delete;

    /// <summary>
    /// 分析 UTF-8 段落文本並切分出雙向文字切片列表
    /// </summary>
    COMPAT_NODISCARD std::vector<BiDiRun> Analyze(compat::string_view text, BiDiDirection defaultDir = BiDiDirection::LeftToRight) const;

    /// <summary>
    /// 判定段落的基礎文本流向
    /// </summary>
    COMPAT_NODISCARD BiDiDirection DetectBaseDirection(compat::string_view text) const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace text
} // namespace cppui
