/**
 * @file PictureBox.cpp
 * @brief WinForms 風格圖片顯示控制項實作
 */

#include "ui/controls/PictureBox.hpp"
#include "ui/graphics/Graphics.hpp"
#include <algorithm>

namespace cppui {
namespace controls {

PictureBox::PictureBox() = default;

PictureBox::PictureBox(std::shared_ptr<graphics::Bitmap> image)
    : m_image(image) {}

std::shared_ptr<graphics::Bitmap> PictureBox::GetImage() const {
    return m_image;
}

void PictureBox::SetImage(std::shared_ptr<graphics::Bitmap> image) {
    if (m_image != image) {
        m_image = image;
        InvalidateMeasure();
        InvalidateVisual();
    }
}

graphics::PictureBoxSizeMode PictureBox::GetSizeMode() const {
    return m_sizeMode;
}

void PictureBox::SetSizeMode(graphics::PictureBoxSizeMode mode) {
    if (m_sizeMode != mode) {
        m_sizeMode = mode;
        InvalidateMeasure();
        InvalidateVisual();
    }
}

void PictureBox::SetImageLocation(compat::string_view path) {
    m_imageLocation = std::string(path.data(), path.size());
    if (!m_imageLocation.empty()) {
        Load(m_imageLocation);
    }
}

bool PictureBox::Load(compat::string_view path) {
    if (path.empty()) {
        throw std::invalid_argument("PictureBox::Load: path cannot be empty.");
    }
    m_imageLocation = std::string(path.data(), path.size());
    try {
        auto bmp = std::make_shared<graphics::Bitmap>(path);
        SetImage(bmp);
        return true;
    } catch (...) {
        return false;
    }
}

core::Size PictureBox::MeasureOverride(core::Size availableSize) {
    if (!m_image) {
        return core::Size(0.0f, 0.0f);
    }

    float imgW = static_cast<float>(m_image->GetWidth());
    float imgH = static_cast<float>(m_image->GetHeight());

    if (m_sizeMode == graphics::PictureBoxSizeMode::AutoSize) {
        return core::Size(imgW, imgH);
    }

    if (m_sizeMode == graphics::PictureBoxSizeMode::Zoom) {
        if (availableSize.width < imgW || availableSize.height < imgH) {
            float scaleW = availableSize.width > 0.0f ? availableSize.width / imgW : 1.0f;
            float scaleH = availableSize.height > 0.0f ? availableSize.height / imgH : 1.0f;
            float scale = std::min(scaleW, scaleH);
            return core::Size(imgW * scale, imgH * scale);
        }
        return core::Size(imgW, imgH);
    }

    return core::Size(imgW, imgH);
}

void PictureBox::ArrangeOverride(core::Size finalSize) {
    (void)finalSize;
}

void PictureBox::OnRender(rendering::DrawingContext& dc) {
    Control::OnRender(dc);

    if (!m_image) return;

    core::Rect bounds = GetRenderBounds();
    float viewW = bounds.width;
    float viewH = bounds.height;
    if (viewW <= 0.0f || viewH <= 0.0f) return;

    float imgW = static_cast<float>(m_image->GetWidth());
    float imgH = static_cast<float>(m_image->GetHeight());
    if (imgW <= 0.0f || imgH <= 0.0f) return;

    graphics::Graphics g(dc);

    switch (m_sizeMode) {
        case graphics::PictureBoxSizeMode::Normal: {
            core::Rect destRect(0.0f, 0.0f, std::min(viewW, imgW), std::min(viewH, imgH));
            core::Rect srcRect(0.0f, 0.0f, destRect.width, destRect.height);
            g.DrawImage(m_image, destRect, srcRect);
            break;
        }

        case graphics::PictureBoxSizeMode::StretchImage: {
            core::Rect destRect(0.0f, 0.0f, viewW, viewH);
            g.DrawImage(m_image, destRect);
            break;
        }

        case graphics::PictureBoxSizeMode::AutoSize: {
            g.DrawImage(m_image, 0.0f, 0.0f);
            break;
        }

        case graphics::PictureBoxSizeMode::CenterImage: {
            float cx = (viewW - imgW) * 0.5f;
            float cy = (viewH - imgH) * 0.5f;
            core::Rect destRect(cx, cy, imgW, imgH);
            g.DrawImage(m_image, destRect);
            break;
        }

        case graphics::PictureBoxSizeMode::Zoom: {
            float scale = std::min(viewW / imgW, viewH / imgH);
            float drawW = imgW * scale;
            float drawH = imgH * scale;
            float cx = (viewW - drawW) * 0.5f;
            float cy = (viewH - drawH) * 0.5f;
            core::Rect destRect(cx, cy, drawW, drawH);
            g.DrawImage(m_image, destRect);
            break;
        }
    }
}

} // namespace controls
} // namespace cppui
