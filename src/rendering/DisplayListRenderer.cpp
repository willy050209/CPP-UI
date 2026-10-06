#include "ui/rendering/DisplayListRenderer.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

namespace cppui::rendering {

namespace {

inline core::Rect IntersectRect(const core::Rect& a, const core::Rect& b) {
    float left   = std::max(a.Left(), b.Left());
    float top    = std::max(a.Top(), b.Top());
    float right  = std::min(a.Right(), b.Right());
    float bottom = std::min(a.Bottom(), b.Bottom());

    if (right <= left || bottom <= top) {
        return core::Rect(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return core::Rect(left, top, right - left, bottom - top);
}

inline core::Color ApplyOpacity(core::Color c, float opacity) {
    if (opacity >= 1.0f) return c;
    if (opacity <= 0.0f) return core::Color(c.r, c.g, c.b, 0);
    uint8_t a = static_cast<uint8_t>(static_cast<float>(c.a) * opacity);
    return core::Color(c.r, c.g, c.b, a);
}

} // namespace

RenderStats DisplayListRenderer::Render(const DisplayList& displayList, IRenderTarget* target) {
    RenderStats stats;
    if (!target) return stats;

    float targetW = static_cast<float>(target->GetWidth());
    float targetH = static_cast<float>(target->GetHeight());
    if (targetW <= 0.0f || targetH <= 0.0f) return stats;

    // 狀態棧初始化
    std::vector<core::Rect> clipStack;
    clipStack.push_back(core::Rect(0.0f, 0.0f, targetW, targetH));

    std::vector<float> opacityStack;
    opacityStack.push_back(1.0f);

    displayList.ForEach([&](const DrawOpHeader& header, const void* payload) {
        stats.totalOps++;

        core::Rect currentClip = clipStack.back();
        float currentOpacity   = opacityStack.back();

        switch (header.type) {
            case DrawOpType::PushClipRect: {
                const auto* op = static_cast<const PushClipRectOp*>(payload);
                core::Rect newClip = IntersectRect(currentClip, op->rect);
                clipStack.push_back(newClip);
                break;
            }

            case DrawOpType::PopClip: {
                if (clipStack.size() > 1) {
                    clipStack.pop_back();
                }
                break;
            }

            case DrawOpType::PushOpacity: {
                const auto* op = static_cast<const PushOpacityOp*>(payload);
                float newOpacity = std::clamp(currentOpacity * op->opacity, 0.0f, 1.0f);
                opacityStack.push_back(newOpacity);
                break;
            }

            case DrawOpType::PopOpacity: {
                if (opacityStack.size() > 1) {
                    opacityStack.pop_back();
                }
                break;
            }

            case DrawOpType::DrawRectangle: {
                if (currentClip.IsEmpty() || currentOpacity <= 0.0f) {
                    stats.culledOps++;
                    break;
                }

                const auto* op = static_cast<const DrawRectOp*>(payload);
                core::Rect visibleRect = IntersectRect(op->rect, currentClip);
                if (visibleRect.IsEmpty()) {
                    stats.culledOps++;
                    break;
                }

                core::Color finalColor = ApplyOpacity(op->color, currentOpacity);
                target->FillRect(visibleRect, finalColor);
                stats.renderedOps++;
                break;
            }

            case DrawOpType::DrawRoundedRectangle: {
                if (currentClip.IsEmpty() || currentOpacity <= 0.0f) {
                    stats.culledOps++;
                    break;
                }

                const auto* op = static_cast<const DrawRoundedRectOp*>(payload);
                core::Rect visibleRect = IntersectRect(op->rect, currentClip);
                if (visibleRect.IsEmpty()) {
                    stats.culledOps++;
                    break;
                }

                core::Color finalColor = ApplyOpacity(op->color, currentOpacity);

                if (op->radius <= 0.5f) {
                    target->FillRect(visibleRect, finalColor);
                } else {
                    // 軟體光柵化圓角矩形 (直接在 target 像素緩衝區上進行圓形遮罩判斷)
                    uint32_t* pixels = target->GetPixelBuffer();
                    uint32_t stride = target->GetWidth();
                    uint32_t pixelValue = (static_cast<uint32_t>(finalColor.a) << 24) |
                                          (static_cast<uint32_t>(finalColor.r) << 16) |
                                          (static_cast<uint32_t>(finalColor.g) << 8)  |
                                          (static_cast<uint32_t>(finalColor.b));

                    int32_t x0 = static_cast<int32_t>(visibleRect.Left());
                    int32_t y0 = static_cast<int32_t>(visibleRect.Top());
                    int32_t x1 = static_cast<int32_t>(visibleRect.Right());
                    int32_t y1 = static_cast<int32_t>(visibleRect.Bottom());

                    float r = std::min({ op->radius, op->rect.width * 0.5f, op->rect.height * 0.5f });
                    float rSq = r * r;

                    float cxLeft   = op->rect.Left() + r;
                    float cxRight  = op->rect.Right() - r;
                    float cyTop    = op->rect.Top() + r;
                    float cyBottom = op->rect.Bottom() - r;

                    for (int32_t y = y0; y < y1; ++y) {
                        float py = static_cast<float>(y) + 0.5f;
                        for (int32_t x = x0; x < x1; ++x) {
                            float px = static_cast<float>(x) + 0.5f;
                            bool inside = true;

                            if (px < cxLeft && py < cyTop) {
                                float dx = px - cxLeft;
                                float dy = py - cyTop;
                                inside = (dx * dx + dy * dy <= rSq);
                            } else if (px > cxRight && py < cyTop) {
                                float dx = px - cxRight;
                                float dy = py - cyTop;
                                inside = (dx * dx + dy * dy <= rSq);
                            } else if (px < cxLeft && py > cyBottom) {
                                float dx = px - cxLeft;
                                float dy = py - cyBottom;
                                inside = (dx * dx + dy * dy <= rSq);
                            } else if (px > cxRight && py > cyBottom) {
                                float dx = px - cxRight;
                                float dy = py - cyBottom;
                                inside = (dx * dx + dy * dy <= rSq);
                            }

                            if (inside) {
                                pixels[static_cast<size_t>(y) * stride + x] = pixelValue;
                            }
                        }
                    }
                }
                stats.renderedOps++;
                break;
            }

            case DrawOpType::DrawLine: {
                if (currentClip.IsEmpty() || currentOpacity <= 0.0f) {
                    stats.culledOps++;
                    break;
                }

                const auto* op = static_cast<const DrawLineOp*>(payload);
                core::Color finalColor = ApplyOpacity(op->color, currentOpacity);

                // 水平或垂直直線最佳化為 FillRect
                if (std::abs(op->p1.y - op->p2.y) < 0.001f) {
                    float left = std::min(op->p1.x, op->p2.x);
                    float w    = std::abs(op->p2.x - op->p1.x);
                    float top  = op->p1.y - op->strokeWidth * 0.5f;
                    core::Rect lineRect(left, top, w, op->strokeWidth);
                    core::Rect visibleRect = IntersectRect(lineRect, currentClip);
                    if (!visibleRect.IsEmpty()) {
                        target->FillRect(visibleRect, finalColor);
                    }
                } else if (std::abs(op->p1.x - op->p2.x) < 0.001f) {
                    float top  = std::min(op->p1.y, op->p2.y);
                    float h    = std::abs(op->p2.y - op->p1.y);
                    float left = op->p1.x - op->strokeWidth * 0.5f;
                    core::Rect lineRect(left, top, op->strokeWidth, h);
                    core::Rect visibleRect = IntersectRect(lineRect, currentClip);
                    if (!visibleRect.IsEmpty()) {
                        target->FillRect(visibleRect, finalColor);
                    }
                } else {
                    // Bresenham 傾斜線段光柵化
                    uint32_t* pixels = target->GetPixelBuffer();
                    uint32_t stride = target->GetWidth();
                    uint32_t pixelValue = (static_cast<uint32_t>(finalColor.a) << 24) |
                                          (static_cast<uint32_t>(finalColor.r) << 16) |
                                          (static_cast<uint32_t>(finalColor.g) << 8)  |
                                          (static_cast<uint32_t>(finalColor.b));

                    int32_t x0 = static_cast<int32_t>(op->p1.x);
                    int32_t y0 = static_cast<int32_t>(op->p1.y);
                    int32_t x1 = static_cast<int32_t>(op->p2.x);
                    int32_t y1 = static_cast<int32_t>(op->p2.y);

                    int32_t dx = std::abs(x1 - x0);
                    int32_t dy = -std::abs(y1 - y0);
                    int32_t sx = (x0 < x1) ? 1 : -1;
                    int32_t sy = (y0 < y1) ? 1 : -1;
                    int32_t err = dx + dy;

                    int32_t clipLeft   = static_cast<int32_t>(currentClip.Left());
                    int32_t clipTop    = static_cast<int32_t>(currentClip.Top());
                    int32_t clipRight  = static_cast<int32_t>(currentClip.Right());
                    int32_t clipBottom = static_cast<int32_t>(currentClip.Bottom());

                    while (true) {
                        if (x0 >= clipLeft && x0 < clipRight && y0 >= clipTop && y0 < clipBottom) {
                            pixels[static_cast<size_t>(y0) * stride + x0] = pixelValue;
                        }
                        if (x0 == x1 && y0 == y1) break;
                        int32_t e2 = 2 * err;
                        if (e2 >= dy) { err += dy; x0 += sx; }
                        if (e2 <= dx) { err += dx; y0 += sy; }
                    }
                }

                stats.renderedOps++;
                break;
            }

            default:
                break;
        }
    });

    return stats;
}

} // namespace cppui::rendering
