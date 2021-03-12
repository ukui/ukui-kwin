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

#ifndef SHADOWHELPER_H
#define SHADOWHELPER_H

#include <QMap>
#include <QSharedPointer>
#include <QPixmap>
#include <KDecoration2/DecorationShadow>

namespace UKUI {

class Decoration;

class ShadowIndex {
public:
    ShadowIndex(const QColor &color, int topLeft, int topRight, int bottomLeft, int bottomRight, qreal darkness, int borderWidth);
    bool operator ==(const ShadowIndex &index);
    bool operator <(const ShadowIndex &index) const;

    QColor color() const;
    int topLeft() const;
    int topRight() const;
    int bottomLeft() const;
    int bottomRight() const;
    qreal darkness() const;
    int borderWidth() const;

private:
    QColor m_color;
    int m_topLeft = -1;
    int m_topRight = -1;
    int m_bottomLeft = -1;
    int m_bottomRight = -1;
    qreal m_darkness = -1;
    int m_borderWidth = -1;
};

class ShadowHelper
{
    friend class Decoration;
public:
    enum State {
        Active,
        Inactive,
    };

    static ShadowHelper *globalInstance();

    void releaseShadows();

    QSharedPointer<KDecoration2::DecorationShadow> getShadow(const ShadowIndex &index);

private:
    ShadowHelper();
    QPixmap getShadowPixmap(const QColor &color,
                            int shadow_border,
                            qreal darkness,
                            int borderRadiusTopLeft = 0,
                            int borderRadiusTopRight = 0,
                            int borderRadiusBottomLeft = 0,
                            int borderRadiusBottomRight = 0);

    QMap<QList<int>, QSharedPointer<KDecoration2::DecorationShadow>> m_inactiveShadowsCache;
    QMap<QList<int>, QSharedPointer<KDecoration2::DecorationShadow>> m_activeShadowsCache;

    QMap<ShadowIndex, QSharedPointer<KDecoration2::DecorationShadow>> m_shadowsCache;

    QPainterPath caculateRelativePainterPath(qreal borderRadiusTopLeft = 0,
                                             qreal borderRadiusTopRight = 0,
                                             qreal borderRadiusBottomLeft = 0,
                                             qreal borderRadiusBottomRight = 0);
};

}

#endif // SHADOWHELPER_H
