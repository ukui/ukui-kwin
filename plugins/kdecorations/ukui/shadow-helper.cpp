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

#define INNERRECT_WIDTH 100

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

using namespace UKUI;

static ShadowHelper *global_instance = nullptr;

ShadowHelper *ShadowHelper::globalInstance()
{
    if (!global_instance)
        global_instance = new ShadowHelper;
    return global_instance;
}

QSharedPointer<KDecoration2::DecorationShadow> ShadowHelper::getShadow(ShadowHelper::State state, int shadow_border, qreal darkness, int borderRadiusTopLeft, int borderRadiusTopRight, int borderRadiusBottomLeft, int borderRadiusBottomRight)
{
    QList<int> unityBorderRadius;
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
    unityBorderRadius<<borderRadiusTopLeft<<borderRadiusTopRight<<borderRadiusBottomLeft<<borderRadiusBottomRight;

    switch (state) {
    case Active: {
        if (auto shadow = m_activeShadowsCache.value(unityBorderRadius)) {
            return shadow;
        }

        auto shadow = QSharedPointer<KDecoration2::DecorationShadow>::create();
        auto pix = this->getShadowPixmap(state, shadow_border, darkness, borderRadiusTopLeft, borderRadiusTopRight, borderRadiusBottomLeft, borderRadiusBottomRight);
        auto img = pix.toImage();
        shadow->setShadow(img);
        int maxTopRadius = qMax(borderRadiusTopLeft, borderRadiusTopRight);
        int maxBottomRadius = qMax(borderRadiusBottomLeft, borderRadiusBottomRight);
        int maxRadius = qMax(maxTopRadius, maxBottomRadius);
        QRect innerRect = QRect(shadow_border + maxRadius, shadow_border + maxRadius, INNERRECT_WIDTH, INNERRECT_WIDTH);
        shadow->setInnerShadowRect(innerRect);
        shadow->setPadding(QMargins(shadow_border, shadow_border, shadow_border, shadow_border));
        m_activeShadowsCache.insert(unityBorderRadius, shadow);
        return shadow;
    }
    case Inactive: {
        if (auto shadow = m_inactiveShadowsCache.value(unityBorderRadius)) {
            return shadow;
        }

        auto shadow = QSharedPointer<KDecoration2::DecorationShadow>::create();
        auto pix = this->getShadowPixmap(state, shadow_border, darkness, borderRadiusTopLeft, borderRadiusTopRight, borderRadiusBottomLeft, borderRadiusBottomRight);
        auto img = pix.toImage();
        shadow->setShadow(img);
        int maxTopRadius = qMax(borderRadiusTopLeft, borderRadiusTopRight);
        int maxBottomRadius = qMax(borderRadiusBottomLeft, borderRadiusBottomRight);
        int maxRadius = qMax(maxTopRadius, maxBottomRadius);
        QRect innerRect = QRect(shadow_border + maxRadius, shadow_border + maxRadius, INNERRECT_WIDTH, INNERRECT_WIDTH);
        shadow->setInnerShadowRect(innerRect);
        shadow->setPadding(QMargins(shadow_border, shadow_border, shadow_border, shadow_border));
        m_inactiveShadowsCache.insert(unityBorderRadius, shadow);
        return shadow;
        break;
    }
    default:
        auto shadow = QSharedPointer<KDecoration2::DecorationShadow>::create();
        return shadow;
    }
}

ShadowHelper::ShadowHelper()
{

}

QPixmap ShadowHelper::getShadowPixmap(ShadowHelper::State state, int shadow_border, qreal darkness, int borderRadiusTopLeft, int borderRadiusTopRight, int borderRadiusBottomLeft, int borderRadiusBottomRight)
{
    int maxTopRadius = qMax(borderRadiusTopLeft, borderRadiusTopRight);
    int maxBottomRadius = qMax(borderRadiusBottomLeft, borderRadiusBottomRight);
    int maxRadius = qMax(maxTopRadius, maxBottomRadius);
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
    painter.fillPath(windowRelativePath, Qt::black);
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
    return darkerTarget;
}
