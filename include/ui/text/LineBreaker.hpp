#pragma once

#include <vector>
#include <memory>
#include "compat/compat.hpp"

namespace cppui {
namespace text {

/// <summary>
/// Unicode 換行斷詞機會等級 (UAX #14)
/// </summary>
enum class BreakOpportunity : uint8_t {
    None      = 0, // 禁止斷行 (例如多字節 UTF-8 中間或特定標點前方)
    CanBreak  = 1, // 允許軟折行 (例如空格、中文字元邊界、連字號)
    MustBreak = 2  // 強制硬換行 (例如 \n 或 \r\n)
};

/// <summary>
/// 單一換行點資料
/// </summary>
struct LineBreak {
    size_t           position    = 0;
    BreakOpportunity opportunity = BreakOpportunity::None;
};

/// <summary>
/// Unicode 換行分析器 (LineBreaker)
/// </summary>
class LineBreaker {
public:
    LineBreaker();
    ~LineBreaker();

    // 禁止複製
    LineBreaker(const LineBreaker&) = delete;
    LineBreaker& operator=(const LineBreaker&) = delete;

    /// <summary>
    /// 計算 UTF-8 字串中的所有斷行機會點
    /// </summary>
    COMPAT_NODISCARD std::vector<LineBreak> FindBreaks(compat::string_view text) const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace text
} // namespace cppui
