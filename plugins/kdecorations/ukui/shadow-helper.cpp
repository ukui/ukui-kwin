/*
 * KWin Style UKUI
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "shadow-helper.h"

#include <QPainter>
#include <QPainterPath>
#include <QPoint>
#include <QImage>

#include <QDebug>

#define INNERRECT_WIDTH 1

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

using namespace UKUI;

static ShadowHelper *global_instance = nullptr;

ShadowHelper *ShadowHelper::globalInstance()
{
    if (!global_instance)
        global_instance = new ShadowHelper;
    return global_instance;
}

void ShadowHelper::releaseShadows()
{
    for (auto shadow : m_shadowsCache) {
        shadow.clear();
    }
    m_shadowsCache.clear();
}

QSharedPointer<KDecoration2::DecorationShadow> ShadowHelper::getShadow(const ShadowIndex &index)
{
    int borderRadiusTopLeft = index.topLeft();
    int borderRadiusTopRight = index.topRight();
    int borderRadiusBottomLeft = index.bottomLeft();
    int borderRadiusBottomRight = index.bottomRight();
    qreal darkness = index.darkness();
    qreal shadow_border = index.borderWidth();

    if (borderRadiusTopLeft < 1) {
        borderRadiusTopLeft = 1;
    }
    if (borderRadiusTopRight < 1) {
        borderRadiusTopRight = 1;
    }
    if (borderRadiusBottomLeft < 1) {
        borderRadiusBottomLeft = 1;
    }
    if (borderRadiusBottomRight < 1) {
        borderRadiusBottomRight = 1;
    }

    auto shadow = m_shadowsCache.value(index);
    if (!shadow.isNull())
        return shadow;

    shadow = QSharedPointer<KDecoration2::DecorationShadow>::create();
    auto pix = this->getShadowPixmap(index.color(), shadow_border, darkness, borderRadiusTopLeft, borderRadiusTopRight, borderRadiusBottomLeft, borderRadiusBottomRight);
    auto img = pix.toImage();
    shadow->setShadow(img);
    int maxTopRadius = qMax(borderRadiusTopLeft, borderRadiusTopRight);
    int maxBottomRadius = qMax(borderRadiusBottomLeft, borderRadiusBottomRight);
    int maxRadius = qMax(maxTopRadius, maxBottomRadius);
    maxRadius = qMax(12, maxRadius);
    QRect innerRect = QRect(shadow_border + maxRadius, shadow_border + maxRadius, INNERRECT_WIDTH, INNERRECT_WIDTH);
    shadow->setInnerShadowRect(innerRect);
    shadow->setPadding(QMargins(shadow_border, shadow_border, shadow_border, shadow_border));

    m_shadowsCache.insert(index, shadow);
    return shadow;
}

ShadowHelper::ShadowHelper()
{

}

QPixmap ShadowHelper::getShadowPixmap(const QColor &color, int shadow_border, qreal darkness, int borderRadiusTopLeft, int borderRadiusTopRight, int borderRadiusBottomLeft, int borderRadiusBottomRight)
{
    int maxTopRadius = qMax(borderRadiusTopLeft, borderRadiusTopRight);
    int maxBottomRadius = qMax(borderRadiusBottomLeft, borderRadiusBottomRight);
    int maxRadius = qMax(maxTopRadius, maxBottomRadius);
    maxRadius = qMax(12, maxRadius);
    QPixmap pix(QSize(2 * maxRadius + 2 * shadow_border + INNERRECT_WIDTH, 2 * maxRadius + 2 * shadow_border + INNERRECT_WIDTH));
    pix.fill(Qt::transparent);

    int squareWidth = 2 * maxRadius + INNERRECT_WIDTH;

    QPainterPath windowRelativePath;
    windowRelativePath.setFillRule(Qt::WindingFill);
    QPoint currentPos;

    // move to top left arc start point
    windowRelativePath.moveTo(borderRadiusTopLeft, 0);
    // top left arc
    auto topLeftBorderRadiusRect = QRect(0, 0, 2 * borderRadiusTopLeft, 2 * borderRadiusTopLeft);
    windowRelativePath.arcTo(topLeftBorderRadiusRect, 90, 90);
    // move to bottom left arc start point
    currentPos = QPoint(0, maxRadius + INNERRECT_WIDTH + maxRadius - borderRadiusBottomLeft);
    //windowRelativePath.moveTo(currentPos);
    // bottom left arc
    auto bottomLeftRect = QRect(0, currentPos.y() - borderRadiusBottomLeft, 2 * borderRadiusBottomLeft, 2 * borderRadiusBottomLeft);
    windowRelativePath.arcTo(bottomLeftRect, 180, 90);
    // move to bottom right arc start point
    currentPos = QPoint(2 * maxRadius + INNERRECT_WIDTH - borderRadiusBottomRight, 2 * maxRadius + INNERRECT_WIDTH);
    //windowRelativePath.moveTo(currentPos);
    // bottom right arc
    auto bottomRightRect = QRect(currentPos.x() - borderRadiusBottomRight, currentPos.y() - 2 * borderRadiusBottomRight, 2 * borderRadiusBottomRight, 2 * borderRadiusBottomRight);
    windowRelativePath.arcTo(bottomRightRect, 270, 90);
    // move to top right arc start point
    currentPos = QPoint(2 * maxRadius + INNERRECT_WIDTH, borderRadiusTopRight);
    //windowRelativePath.moveTo(currentPos);
    // top right arc
    auto topRightRect = QRect(squareWidth - 2 * borderRadiusTopRight, 0, 2 * borderRadiusTopRight, 2 * borderRadiusTopRight);
    windowRelativePath.arcTo(topRightRect, 0, 90);

    QPainter painter(&pix);
    painter.save();
    painter.translate(shadow_border, shadow_border);
    painter.fillPath(windowRelativePath, color);
    painter.restore();

    QImage rawImg = pix.toImage();
    qt_blurImage(rawImg, shadow_border, true, true);

    QPixmap target = QPixmap::fromImage(rawImg);
    QPainter painter2(&target);
    painter2.save();
    painter2.setRenderHint(QPainter::Antialiasing);
    painter2.translate(shadow_border, shadow_border);
    painter2.setCompositionMode(QPainter::CompositionMode_Clear);
    painter2.fillPath(windowRelativePath, Qt::transparent);
    painter2.restore();
    painter2.end();

    // handle darkness
    QImage newImg = target.toImage();
    for (int x = 0; x < newImg.width(); x++) {
        for (int y = 0; y < newImg.height(); y++) {
            auto color = newImg.pixelColor(x, y);
            if (color.alpha() == 0)
                continue;
            color.setAlphaF(darkness * color.alphaF());
            newImg.setPixelColor(x, y, color);
        }
    }

    QPixmap darkerTarget = QPixmap::fromImage(newImg);
    painter2.begin(&darkerTarget);

    auto borderPath = caculateRelativePainterPath(borderRadiusTopLeft + 0.5, borderRadiusTopRight + 0.5, borderRadiusBottomLeft + 0.5, borderRadiusBottomRight + 0.5);
    painter2.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter2.setRenderHint(QPainter::HighQualityAntialiasing);
    QColor borderColor = color;
    borderColor.setAlphaF(0.05);
    painter2.setPen(QPen(borderColor, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter2.setBrush(Qt::NoBrush);
    painter2.translate(shadow_border, shadow_border);
    painter2.translate(-0.5, -0.5);
    painter2.drawPath(borderPath);

    return darkerTarget;
}

QPainterPath ShadowHelper::caculateRelativePainterPath(qreal borderRadiusTopLeft, qreal borderRadiusTopRight, qreal borderRadiusBottomLeft, qreal borderRadiusBottomRight)
{
    qreal maxTopRadius = qMax(borderRadiusTopLeft, borderRadiusTopRight);
    qreal maxBottomRadius = qMax(borderRadiusBottomLeft, borderRadiusBottomRight);
    qreal maxRadius = qMax(maxTopRadius, maxBottomRadius);
    maxRadius = qMax(12.0, maxRadius);

    qreal squareWidth = 2 * maxRadius + INNERRECT_WIDTH;

    QPainterPath windowRelativePath;
    windowRelativePath.setFillRule(Qt::WindingFill);
    QPointF currentPos;

    // move to top left arc start point
    windowRelativePath.moveTo(borderRadiusTopLeft, 0);
    // top left arc
    auto topLeftBorderRadiusRect = QRectF(0, 0, 2 * borderRadiusTopLeft, 2 * borderRadiusTopLeft);
    windowRelativePath.arcTo(topLeftBorderRadiusRect, 90, 90);
    // move to bottom left arc start point
    currentPos = QPointF(0, maxRadius + INNERRECT_WIDTH + maxRadius - borderRadiusBottomLeft);
    //windowRelativePath.moveTo(currentPos);
    // bottom left arc
    auto bottomLeftRect = QRectF(0, currentPos.y() - borderRadiusBottomLeft, 2 * borderRadiusBottomLeft, 2 * borderRadiusBottomLeft);
    windowRelativePath.arcTo(bottomLeftRect, 180, 90);
    // move to bottom right arc start point
    currentPos = QPointF(2 * maxRadius + INNERRECT_WIDTH - borderRadiusBottomRight, 2 * maxRadius + INNERRECT_WIDTH);
    //windowRelativePath.moveTo(currentPos);
    // bottom right arc
    auto bottomRightRect = QRectF(currentPos.x() - borderRadiusBottomRight, currentPos.y() - 2 * borderRadiusBottomRight, 2 * borderRadiusBottomRight, 2 * borderRadiusBottomRight);
    windowRelativePath.arcTo(bottomRightRect, 270, 90);
    // move to top right arc start point
    currentPos = QPointF(2 * maxRadius + INNERRECT_WIDTH, borderRadiusTopRight);
    //windowRelativePath.moveTo(currentPos);
    // top right arc
    auto topRightRect = QRectF(squareWidth - 2 * borderRadiusTopRight, 0, 2 * borderRadiusTopRight, 2 * borderRadiusTopRight);
    windowRelativePath.arcTo(topRightRect, 0, 90);

    return windowRelativePath;
}

ShadowIndex::ShadowIndex(const QColor &color, int topLeft, int topRight, int bottomLeft, int bottomRight, qreal darkness, int borderWidth)
{
    m_color = color;
    m_topLeft = topLeft;
    m_topRight = topRight;
    m_bottomLeft = bottomLeft;
    m_bottomRight = bottomRight;
    m_darkness = darkness;
    m_borderWidth = borderWidth;
}

bool ShadowIndex::operator ==(const ShadowIndex &index)
{
    if (m_color != index.m_color) {
        return false;
    }
    if (m_topLeft != index.m_topLeft) {
        return false;
    }
    if (m_topRight != index.m_topRight) {
        return false;
    }
    if (m_bottomLeft != index.m_bottomLeft) {
        return false;
    }
    if (m_bottomRight != index.m_bottomRight) {
        return false;
    }
    if (m_darkness != index.m_darkness) {
        return false;
    }
    if (m_borderWidth != index.m_borderWidth) {
        return false;
    }
    return true;
}

bool ShadowIndex::operator <(const ShadowIndex &index) const
{
    return m_topLeft < index.m_topLeft;
}

QColor ShadowIndex::color() const
{
    return m_color;
}


int ShadowIndex::topLeft() const
{
    return m_topLeft;
}

int ShadowIndex::topRight() const
{
    return m_topRight;
}

int ShadowIndex::bottomLeft() const
{
    return m_bottomLeft;
}

int ShadowIndex::bottomRight() const
{
    return m_bottomRight;
}

qreal ShadowIndex::darkness() const
{
    return m_darkness;
}

int ShadowIndex::borderWidth() const
{
    return m_borderWidth;
}
