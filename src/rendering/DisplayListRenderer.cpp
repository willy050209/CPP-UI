#include "ui/rendering/DisplayListRenderer.hpp"
#include "compat/Compat.hpp"
#include "ui/graphics/Bitmap.hpp"
#include "ui/graphics/GraphicsPath.hpp"
#include "ui/graphics/Brush.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

#if defined(CPPUI_HAS_SKIA) && CPPUI_HAS_SKIA
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkColor.h"
#include "include/core/SkPath.h"
#include "include/core/SkImage.h"
#include "include/effects/SkGradientShader.h"
#endif

#ifdef DrawText
#undef DrawText
#endif

namespace cppui {
namespace rendering {

namespace {

COMPAT_NODISCARD inline COMPAT_CONSTEXPR_14 core::Rect IntersectRect(const core::Rect& a, const core::Rect& b) {
    float left   = std::max(a.Left(), b.Left());
    float top    = std::max(a.Top(), b.Top());
    float right  = std::min(a.Right(), b.Right());
    float bottom = std::min(a.Bottom(), b.Bottom());

    if (right <= left || bottom <= top) {
        return core::Rect(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return core::Rect(left, top, right - left, bottom - top);
}

COMPAT_NODISCARD inline COMPAT_CONSTEXPR_14 core::Color ApplyOpacity(core::Color c, float opacity) {
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

#if defined(CPPUI_HAS_SKIA) && CPPUI_HAS_SKIA
    if (void* canvasPtr = target->GetSkCanvas()) {
        auto* skCanvas = static_cast<SkCanvas*>(canvasPtr);
        std::vector<core::Rect> clipStack;
        clipStack.push_back(core::Rect(0.0f, 0.0f, targetW, targetH));

        displayList.ForEach([&](const DrawOpHeader& header, const void* payload) {
            stats.totalOps++;
            switch (header.type) {
                case DrawOpType::PushClipRect: {
                    const auto* op = static_cast<const PushClipRectOp*>(payload);
                    core::Rect newClip = IntersectRect(clipStack.back(), op->rect);
                    clipStack.push_back(newClip);
                    skCanvas->save();
                    skCanvas->clipRect(SkRect::MakeXYWH(op->rect.x, op->rect.y, op->rect.width, op->rect.height), SkClipOp::kIntersect, true);
                    break;
                }
                case DrawOpType::PopClip: {
                    if (clipStack.size() > 1) {
                        clipStack.pop_back();
                    }
                    skCanvas->restore();
                    break;
                }
                case DrawOpType::PushOpacity: {
                    const auto* op = static_cast<const PushOpacityOp*>(payload);
                    skCanvas->saveLayerAlphaf(nullptr, op->opacity);
                    break;
                }
                case DrawOpType::PopOpacity: {
                    skCanvas->restore();
                    break;
                }
                case DrawOpType::DrawRectangle: {
                    const auto* op = static_cast<const DrawRectOp*>(payload);
                    SkPaint paint;
                    paint.setColor(SkColorSetARGB(op->color.a, op->color.r, op->color.g, op->color.b));
                    paint.setStyle(SkPaint::kFill_Style);
                    paint.setAntiAlias(true);
                    skCanvas->drawRect(SkRect::MakeXYWH(op->rect.x, op->rect.y, op->rect.width, op->rect.height), paint);
                    stats.renderedOps++;
                    break;
                }
                case DrawOpType::DrawRoundedRectangle: {
                    const auto* op = static_cast<const DrawRoundedRectOp*>(payload);
                    SkPaint paint;
                    paint.setColor(SkColorSetARGB(op->color.a, op->color.r, op->color.g, op->color.b));
                    paint.setStyle(SkPaint::kFill_Style);
                    paint.setAntiAlias(true);
                    skCanvas->drawRRect(SkRRect::MakeRectXY(SkRect::MakeXYWH(op->rect.x, op->rect.y, op->rect.width, op->rect.height), op->radius, op->radius), paint);
                    stats.renderedOps++;
                    break;
                }
                case DrawOpType::DrawLine: {
                    const auto* op = static_cast<const DrawLineOp*>(payload);
                    SkPaint paint;
                    paint.setColor(SkColorSetARGB(op->color.a, op->color.r, op->color.g, op->color.b));
                    paint.setStrokeWidth(op->strokeWidth);
                    paint.setStyle(SkPaint::kStroke_Style);
                    paint.setAntiAlias(true);
                    skCanvas->drawLine(op->p0.x, op->p0.y, op->p1.x, op->p1.y, paint);
                    stats.renderedOps++;
                    break;
                }
                case DrawOpType::DrawText: {
                    const auto* op = static_cast<const DrawTextOp*>(payload);
                    const char* extraData = reinterpret_cast<const char*>(op) + sizeof(DrawTextOp);
                    compat::string_view family(extraData, op->familyLength);
                    compat::string_view text(extraData + op->familyLength, op->textLength);
                    target->DrawText(text, op->origin, family, op->fontSize, op->fontWeight, op->fontStyle, op->color, clipStack.back());
                    stats.renderedOps++;
                    break;
                }
                case DrawOpType::DrawTextLayout: {
                    const auto* op = static_cast<const DrawTextLayoutOp*>(payload);
                    target->DrawTextLayout(op->nativeLayoutHandle, op->origin, op->color, clipStack.back());
                    stats.renderedOps++;
                    break;
                }
                case DrawOpType::DrawEllipse: {
                    const auto* op = static_cast<const DrawEllipseOp*>(payload);
                    SkPaint paint;
                    paint.setColor(SkColorSetARGB(op->color.a, op->color.r, op->color.g, op->color.b));
                    paint.setStrokeWidth(op->strokeWidth);
                    paint.setStyle(SkPaint::kStroke_Style);
                    paint.setAntiAlias(true);
                    skCanvas->drawOval(SkRect::MakeXYWH(op->rect.x, op->rect.y, op->rect.width, op->rect.height), paint);
                    stats.renderedOps++;
                    break;
                }
                case DrawOpType::FillEllipse: {
                    const auto* op = static_cast<const FillEllipseOp*>(payload);
                    SkPaint paint;
                    paint.setColor(SkColorSetARGB(op->color.a, op->color.r, op->color.g, op->color.b));
                    paint.setStyle(SkPaint::kFill_Style);
                    paint.setAntiAlias(true);
                    skCanvas->drawOval(SkRect::MakeXYWH(op->rect.x, op->rect.y, op->rect.width, op->rect.height), paint);
                    stats.renderedOps++;
                    break;
                }
                case DrawOpType::DrawPath: {
                    const auto* op = static_cast<const DrawPathOp*>(payload);
                    if (op->pointCount >= 2) {
                        const auto* points = reinterpret_cast<const core::Point*>(reinterpret_cast<const char*>(op) + sizeof(DrawPathOp));
                        SkPaint paint;
                        paint.setColor(SkColorSetARGB(op->color.a, op->color.r, op->color.g, op->color.b));
                        paint.setStrokeWidth(op->strokeWidth);
                        paint.setStyle(SkPaint::kStroke_Style);
                        paint.setAntiAlias(true);
                        SkPath skPath;
                        skPath.moveTo(points[0].x, points[0].y);
                        for (size_t i = 1; i < op->pointCount; ++i) {
                            skPath.lineTo(points[i].x, points[i].y);
                        }
                        skCanvas->drawPath(skPath, paint);
                    }
                    stats.renderedOps++;
                    break;
                }
                case DrawOpType::FillPath: {
                    const auto* op = static_cast<const FillPathOp*>(payload);
                    if (op->pointCount >= 3) {
                        const auto* points = reinterpret_cast<const core::Point*>(reinterpret_cast<const char*>(op) + sizeof(FillPathOp));
                        SkPaint paint;
                        paint.setStyle(SkPaint::kFill_Style);
                        paint.setAntiAlias(true);
                        if (op->hasGradient) {
                            SkPoint pts[2] = {
                                SkPoint::Make(op->gradStart.x, op->gradStart.y),
                                SkPoint::Make(op->gradEnd.x, op->gradEnd.y)
                            };
                            SkColor colors[2] = {
                                SkColorSetARGB(op->gradStartColor.a, op->gradStartColor.r, op->gradStartColor.g, op->gradStartColor.b),
                                SkColorSetARGB(op->gradEndColor.a, op->gradEndColor.r, op->gradEndColor.g, op->gradEndColor.b)
                            };
                            paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp));
                        } else {
                            paint.setColor(SkColorSetARGB(op->color.a, op->color.r, op->color.g, op->color.b));
                        }
                        SkPath skPath;
                        skPath.setFillType(op->fillMode == 0 ? SkPathFillType::kEvenOdd : SkPathFillType::kWinding);
                        skPath.moveTo(points[0].x, points[0].y);
                        for (size_t i = 1; i < op->pointCount; ++i) {
                            skPath.lineTo(points[i].x, points[i].y);
                        }
                        skPath.close();
                        skCanvas->drawPath(skPath, paint);
                    }
                    stats.renderedOps++;
                    break;
                }
                case DrawOpType::DrawImage: {
                    const auto* op = static_cast<const DrawImageOp*>(payload);
                    if (op->bitmapPtr) {
                        const auto* bmp = static_cast<const graphics::Bitmap*>(op->bitmapPtr);
                        SkImageInfo info = SkImageInfo::MakeN32Premul(bmp->GetWidth(), bmp->GetHeight());
                        auto span = bmp->GetPixelSpan();
                        SkPixmap pixmap(info, span.data(), bmp->GetStride());
                        sk_sp<SkImage> skImg = SkImages::RasterFromPixmapCopy(pixmap);
                        if (skImg) {
                            SkPaint paint;
                            paint.setAlphaf(op->opacity);
                            SkRect dst = SkRect::MakeXYWH(op->destRect.x, op->destRect.y, op->destRect.width, op->destRect.height);
                            SkRect src = SkRect::MakeXYWH(op->srcRect.x, op->srcRect.y, op->srcRect.width, op->srcRect.height);
                            skCanvas->drawImageRect(skImg, src, dst, SkSamplingOptions(SkFilterMode::kLinear), &paint, SkCanvas::kFast_SrcRectConstraint);
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
#endif

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
                float newOpacity = compat::clamp(currentOpacity * op->opacity, 0.0f, 1.0f);
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

            case DrawOpType::DrawText: {
                if (currentClip.IsEmpty() || currentOpacity <= 0.0f) {
                    stats.culledOps++;
                    break;
                }
                const auto* op = static_cast<const DrawTextOp*>(payload);
                const char* extraData = reinterpret_cast<const char*>(op) + sizeof(DrawTextOp);
                compat::string_view family(extraData, op->familyLength);
                compat::string_view text(extraData + op->familyLength, op->textLength);

                core::Color finalColor = ApplyOpacity(op->color, currentOpacity);
                target->DrawText(text, op->origin, family, op->fontSize, op->fontWeight, op->fontStyle, finalColor, currentClip);
                stats.renderedOps++;
                break;
            }

            case DrawOpType::DrawTextLayout: {
                if (currentClip.IsEmpty() || currentOpacity <= 0.0f) {
                    stats.culledOps++;
                    break;
                }
                const auto* op = static_cast<const DrawTextLayoutOp*>(payload);
                core::Color finalColor = ApplyOpacity(op->color, currentOpacity);
                target->DrawTextLayout(op->nativeLayoutHandle, op->origin, finalColor, currentClip);
                stats.renderedOps++;
                break;
            }

            case DrawOpType::DrawEllipse: {
                if (currentClip.IsEmpty() || currentOpacity <= 0.0f) {
                    stats.culledOps++;
                    break;
                }
                const auto* op = static_cast<const DrawEllipseOp*>(payload);
                core::Rect visibleRect = IntersectRect(op->rect, currentClip);
                if (visibleRect.IsEmpty()) {
                    stats.culledOps++;
                    break;
                }
                core::Color finalColor = ApplyOpacity(op->color, currentOpacity);
                uint32_t* pixels = target->GetPixelBuffer();
                uint32_t stride = target->GetWidth();
                uint32_t pixelValue = finalColor.ToArgb();

                float cx = op->rect.x + op->rect.width * 0.5f;
                float cy = op->rect.y + op->rect.height * 0.5f;
                float rx = op->rect.width * 0.5f;
                float ry = op->rect.height * 0.5f;
                constexpr int32_t steps = 64;
                for (int32_t i = 0; i < steps; ++i) {
                    float theta1 = (static_cast<float>(i) * 2.0f * 3.14159265f) / steps;
                    float theta2 = (static_cast<float>(i + 1) * 2.0f * 3.14159265f) / steps;
                    int32_t x1 = static_cast<int32_t>(cx + rx * std::cos(theta1));
                    int32_t y1 = static_cast<int32_t>(cy + ry * std::sin(theta1));
                    int32_t x2 = static_cast<int32_t>(cx + rx * std::cos(theta2));
                    int32_t y2 = static_cast<int32_t>(cy + ry * std::sin(theta2));

                    int32_t dx = std::abs(x2 - x1);
                    int32_t dy = -std::abs(y2 - y1);
                    int32_t sx = (x1 < x2) ? 1 : -1;
                    int32_t sy = (y1 < y2) ? 1 : -1;
                    int32_t err = dx + dy;
                    int32_t cl = static_cast<int32_t>(currentClip.Left());
                    int32_t ct = static_cast<int32_t>(currentClip.Top());
                    int32_t cr = static_cast<int32_t>(currentClip.Right());
                    int32_t cb = static_cast<int32_t>(currentClip.Bottom());

                    while (true) {
                        if (x1 >= cl && x1 < cr && y1 >= ct && y1 < cb) {
                            pixels[static_cast<size_t>(y1) * stride + x1] = pixelValue;
                        }
                        if (x1 == x2 && y1 == y2) break;
                        int32_t e2 = 2 * err;
                        if (e2 >= dy) { err += dy; x1 += sx; }
                        if (e2 <= dx) { err += dx; y1 += sy; }
                    }
                }
                stats.renderedOps++;
                break;
            }

            case DrawOpType::FillEllipse: {
                if (currentClip.IsEmpty() || currentOpacity <= 0.0f) {
                    stats.culledOps++;
                    break;
                }
                const auto* op = static_cast<const FillEllipseOp*>(payload);
                core::Rect visibleRect = IntersectRect(op->rect, currentClip);
                if (visibleRect.IsEmpty()) {
                    stats.culledOps++;
                    break;
                }
                core::Color finalColor = ApplyOpacity(op->color, currentOpacity);
                uint32_t* pixels = target->GetPixelBuffer();
                uint32_t stride = target->GetWidth();
                uint32_t pixelValue = finalColor.ToArgb();

                float cx = op->rect.x + op->rect.width * 0.5f;
                float cy = op->rect.y + op->rect.height * 0.5f;
                float rx = op->rect.width * 0.5f;
                float ry = op->rect.height * 0.5f;
                if (rx <= 0.0f || ry <= 0.0f) break;

                int32_t y0 = static_cast<int32_t>(visibleRect.Top());
                int32_t y1 = static_cast<int32_t>(visibleRect.Bottom());

                for (int32_t y = y0; y < y1; ++y) {
                    float py = static_cast<float>(y) + 0.5f;
                    float dy = py - cy;
                    float ratio = 1.0f - (dy * dy) / (ry * ry);
                    if (ratio >= 0.0f) {
                        float dx = rx * std::sqrt(ratio);
                        int32_t xStart = std::max(static_cast<int32_t>(cx - dx), static_cast<int32_t>(visibleRect.Left()));
                        int32_t xEnd   = std::min(static_cast<int32_t>(cx + dx + 1.0f), static_cast<int32_t>(visibleRect.Right()));
                        for (int32_t x = xStart; x < xEnd; ++x) {
                            pixels[static_cast<size_t>(y) * stride + x] = pixelValue;
                        }
                    }
                }
                stats.renderedOps++;
                break;
            }

            case DrawOpType::DrawPath: {
                if (currentClip.IsEmpty() || currentOpacity <= 0.0f) {
                    stats.culledOps++;
                    break;
                }
                const auto* op = static_cast<const DrawPathOp*>(payload);
                if (op->pointCount < 2) { stats.culledOps++; break; }

                const auto* points = reinterpret_cast<const core::Point*>(reinterpret_cast<const char*>(op) + sizeof(DrawPathOp));
                core::Color finalColor = ApplyOpacity(op->color, currentOpacity);
                uint32_t* pixels = target->GetPixelBuffer();
                uint32_t stride = target->GetWidth();
                uint32_t pixelValue = finalColor.ToArgb();

                int32_t cl = static_cast<int32_t>(currentClip.Left());
                int32_t ct = static_cast<int32_t>(currentClip.Top());
                int32_t cr = static_cast<int32_t>(currentClip.Right());
                int32_t cb = static_cast<int32_t>(currentClip.Bottom());

                for (size_t i = 0; i + 1 < op->pointCount; ++i) {
                    int32_t x0 = static_cast<int32_t>(points[i].x);
                    int32_t y0 = static_cast<int32_t>(points[i].y);
                    int32_t x1 = static_cast<int32_t>(points[i + 1].x);
                    int32_t y1 = static_cast<int32_t>(points[i + 1].y);

                    int32_t dx = std::abs(x1 - x0);
                    int32_t dy = -std::abs(y1 - y0);
                    int32_t sx = (x0 < x1) ? 1 : -1;
                    int32_t sy = (y0 < y1) ? 1 : -1;
                    int32_t err = dx + dy;

                    while (true) {
                        if (x0 >= cl && x0 < cr && y0 >= ct && y0 < cb) {
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

            case DrawOpType::FillPath: {
                if (currentClip.IsEmpty() || currentOpacity <= 0.0f) {
                    stats.culledOps++;
                    break;
                }
                const auto* op = static_cast<const FillPathOp*>(payload);
                if (op->pointCount < 3) { stats.culledOps++; break; }

                core::Rect visibleRect = IntersectRect(op->bounds, currentClip);
                if (visibleRect.IsEmpty()) { stats.culledOps++; break; }

                const auto* points = reinterpret_cast<const core::Point*>(reinterpret_cast<const char*>(op) + sizeof(FillPathOp));
                core::Color finalColor = ApplyOpacity(op->color, currentOpacity);
                uint32_t* pixels = target->GetPixelBuffer();
                uint32_t stride = target->GetWidth();
                uint32_t pixelValue = finalColor.ToArgb();

                int32_t y0 = static_cast<int32_t>(visibleRect.Top());
                int32_t y1 = static_cast<int32_t>(visibleRect.Bottom());
                int32_t x0 = static_cast<int32_t>(visibleRect.Left());
                int32_t x1 = static_cast<int32_t>(visibleRect.Right());

                float gdx = op->gradEnd.x - op->gradStart.x;
                float gdy = op->gradEnd.y - op->gradStart.y;
                float glenSq = gdx * gdx + gdy * gdy;
                if (glenSq <= 0.00001f) glenSq = 1.0f;

                for (int32_t y = y0; y < y1; ++y) {
                    float py = static_cast<float>(y) + 0.5f;
                    for (int32_t x = x0; x < x1; ++x) {
                        float px = static_cast<float>(x) + 0.5f;

                        // 射線交叉點計算 (Ray-casting against inline points)
                        int32_t crossings = 0;
                        for (size_t i = 0, j = op->pointCount - 1; i < op->pointCount; j = i++) {
                            const auto& p1 = points[i];
                            const auto& p2 = points[j];
                            if (((p1.y > py) != (p2.y > py)) &&
                                (px < (p2.x - p1.x) * (py - p1.y) / (p2.y - p1.y + 0.000001f) + p1.x)) {
                                crossings++;
                            }
                        }

                        if ((crossings % 2) != 0) {
                            if (op->hasGradient) {
                                float t = ((px - op->gradStart.x) * gdx + (py - op->gradStart.y) * gdy) / glenSq;
                                t = compat::clamp(t, 0.0f, 1.0f);
                                uint8_t a = static_cast<uint8_t>(static_cast<float>(op->gradStartColor.a) + t * (static_cast<float>(op->gradEndColor.a) - static_cast<float>(op->gradStartColor.a)));
                                uint8_t r = static_cast<uint8_t>(static_cast<float>(op->gradStartColor.r) + t * (static_cast<float>(op->gradEndColor.r) - static_cast<float>(op->gradStartColor.r)));
                                uint8_t g = static_cast<uint8_t>(static_cast<float>(op->gradStartColor.g) + t * (static_cast<float>(op->gradEndColor.g) - static_cast<float>(op->gradStartColor.g)));
                                uint8_t b = static_cast<uint8_t>(static_cast<float>(op->gradStartColor.b) + t * (static_cast<float>(op->gradEndColor.b) - static_cast<float>(op->gradStartColor.b)));
                                core::Color gradCol = ApplyOpacity(core::Color(r, g, b, a), currentOpacity);
                                pixels[static_cast<size_t>(y) * stride + x] = gradCol.ToArgb();
                            } else {
                                pixels[static_cast<size_t>(y) * stride + x] = pixelValue;
                            }
                        }
                    }
                }
                stats.renderedOps++;
                break;
            }

            case DrawOpType::DrawImage: {
                if (currentClip.IsEmpty() || currentOpacity <= 0.0f) {
                    stats.culledOps++;
                    break;
                }
                const auto* op = static_cast<const DrawImageOp*>(payload);
                if (!op->bitmapPtr) { stats.culledOps++; break; }

                const auto* bmp = static_cast<const graphics::Bitmap*>(op->bitmapPtr);
                core::Rect visibleRect = IntersectRect(op->destRect, currentClip);
                if (visibleRect.IsEmpty() || op->destRect.width <= 0.0f || op->destRect.height <= 0.0f) {
                    stats.culledOps++;
                    break;
                }

                uint32_t* pixels = target->GetPixelBuffer();
                uint32_t stride = target->GetWidth();
                float totalOpacity = currentOpacity * op->opacity;

                int32_t y0 = static_cast<int32_t>(visibleRect.Top());
                int32_t y1 = static_cast<int32_t>(visibleRect.Bottom());
                int32_t x0 = static_cast<int32_t>(visibleRect.Left());
                int32_t x1 = static_cast<int32_t>(visibleRect.Right());

                for (int32_t y = y0; y < y1; ++y) {
                    float v = (static_cast<float>(y) - op->destRect.y) / op->destRect.height;
                    float sy_f = op->srcRect.y + v * op->srcRect.height;
                    uint32_t sy = static_cast<uint32_t>(compat::clamp(sy_f, 0.0f, static_cast<float>(bmp->GetHeight() - 1)));

                    for (int32_t x = x0; x < x1; ++x) {
                        float u = (static_cast<float>(x) - op->destRect.x) / op->destRect.width;
                        float sx_f = op->srcRect.x + u * op->srcRect.width;
                        uint32_t sx = static_cast<uint32_t>(compat::clamp(sx_f, 0.0f, static_cast<float>(bmp->GetWidth() - 1)));

                        core::Color srcColor = bmp->GetPixel(sx, sy);
                        srcColor = ApplyOpacity(srcColor, totalOpacity);

                        if (srcColor.a == 255) {
                            pixels[static_cast<size_t>(y) * stride + x] = srcColor.ToArgb();
                        } else if (srcColor.a > 0) {
                            uint32_t bgArgb = pixels[static_cast<size_t>(y) * stride + x];
                            core::Color bgColor = core::Color::FromArgb(bgArgb);
                            float alpha = static_cast<float>(srcColor.a) / 255.0f;
                            float invAlpha = 1.0f - alpha;
                            uint8_t outR = static_cast<uint8_t>(srcColor.r * alpha + bgColor.r * invAlpha);
                            uint8_t outG = static_cast<uint8_t>(srcColor.g * alpha + bgColor.g * invAlpha);
                            uint8_t outB = static_cast<uint8_t>(srcColor.b * alpha + bgColor.b * invAlpha);
                            core::Color outColor(outR, outG, outB, 255);
                            pixels[static_cast<size_t>(y) * stride + x] = outColor.ToArgb();
                        }
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

} // namespace rendering
} // namespace cppui
