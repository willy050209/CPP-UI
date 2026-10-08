/**
 * @file TextLayout.hpp
 * @brief 多行文字排版成果物件，提供文字幾何度量、行資訊與命中測試 (HitTest)
 */

#pragma once

#include <vector>
#include <string>
#include <memory>
#include "compat/compat.hpp"
#include "ui/core/Geometry.hpp"
#include "ui/text/Font.hpp"
#include "ui/text/TextShaper.hpp"

namespace cppui {
namespace text {

/// <summary>
/// 段落水平對齊方式
/// </summary>
enum class TextAlignment : uint8_t {
    Left    = 0,
    Center  = 1,
    Right   = 2,
    Justify = 3
};

/// <summary>
/// 自動折行策略
/// </summary>
enum class TextWrapping : uint8_t {
    NoWrap        = 0, // 不自動折行
    WordWrap      = 1, // 優先依單詞與斷行機會折行
    CharacterWrap = 2  // 字元級硬折行
};

/// <summary>
/// 超出邊界之文字裁剪修剪策略 (Ellipsis)
/// </summary>
enum class TextTrimming : uint8_t {
    None              = 0, // 直接裁切或溢出
    CharacterEllipsis = 1, // 字元級加入省略號 (...)
    WordEllipsis      = 2  // 單詞級加入省略號 (...)
};

/// <summary>
/// 排版產出的單行文字幾何度量資訊 (TextLine)
/// </summary>
struct TextLine {
    size_t     startIndex = 0;    // 在原始字串中的起始索引
    size_t     length     = 0;    // 此行的字串長度
    core::Rect bounds;            // 此行文字在佈局座標系中的包圍盒矩形
    float      baseline   = 0.0f; // 基線 Y 座標 (相對於此行頂部或佈局頂部)
    bool       isWrapped  = false;// 是否為軟折行行
};

/// <summary>
/// 文字點擊與命中測試結果 (TextHitTestResult)
/// </summary>
struct TextHitTestResult {
    size_t     textPosition  = 0;     // 命中的字元索引
    bool       isTrailingHit = false; // 是否命中該字元的後半部 (決定游標繪製於前或後)
    bool       isInside      = false; // 是否精確點擊在文字包圍盒內
    core::Rect cursorRect;            // 推薦的游標繪製矩形 (通常寬 1~2px，高為行高)
};

/// <summary>
/// 完整多行文字排版成果物件 (TextLayout)
/// </summary>
class TextLayout {
public:
    /// <summary>預設建構子。</summary>
    TextLayout();

    /// <summary>解構子。</summary>
    ~TextLayout();

    // 支援拷貝與共享所有權 (O(1) 共享底層排版狀態)
    TextLayout(const TextLayout& other);
    TextLayout& operator=(const TextLayout& other);

    // 支援移動語意
    TextLayout(TextLayout&& other) noexcept;
    TextLayout& operator=(TextLayout&& other) noexcept;

    /// <summary>
    /// 取得排版總體寬高尺寸 (以 DIP 浮點數表示)。
    /// </summary>
    /// <returns>Size 寬高尺寸。</returns>
    COMPAT_NODISCARD core::Size GetSize() const noexcept;

    /// <summary>
    /// 取得總行數。
    /// </summary>
    /// <returns>行數數值。</returns>
    COMPAT_NODISCARD size_t GetLineCount() const noexcept;

    /// <summary>
    /// 取得所有已排版行的切片視圖。
    /// </summary>
    /// <returns>TextLine 唯讀切片視圖。</returns>
    COMPAT_NODISCARD compat::span<const TextLine> GetLines() const noexcept;

    /// <summary>
    /// 依座標執行命中測試，查詢對應之字元索引與游標幾何。
    /// </summary>
    /// <param name="point">點擊命中之局部座標。</param>
    /// <returns>TextHitTestResult 結果結構。</returns>
    COMPAT_NODISCARD TextHitTestResult HitTestPoint(core::Point point) const;

    /// <summary>
    /// 依字元索引查詢其游標幾何矩形 (用於繪製閃爍插入游標)。
    /// </summary>
    /// <param name="textPosition">字元索引。</param>
    /// <returns>游標幾何矩形。</returns>
    COMPAT_NODISCARD core::Rect HitTestTextPosition(size_t textPosition) const;

    /// <summary>
    /// 取得原始排版文字。
    /// </summary>
    /// <returns>文字字串視圖。</returns>
    COMPAT_NODISCARD compat::string_view GetText() const noexcept;

    /// <summary>
    /// 取得字型設定。
    /// </summary>
    /// <returns>FontDescriptor 常數引用。</returns>
    COMPAT_NODISCARD const FontDescriptor& GetFont() const noexcept;

    /// <summary>
    /// 內部或後端專用句柄存取。
    /// </summary>
    /// <returns>原生排版控制代碼。</returns>
    COMPAT_NODISCARD void* GetNativeHandle() const noexcept;

    // 內部建構輔助 (PImpl)
    struct Impl;
    explicit TextLayout(std::shared_ptr<Impl> impl);

private:
    std::shared_ptr<Impl> m_impl;
};

} // namespace text
} // namespace cppui
