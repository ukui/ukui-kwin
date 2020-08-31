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

class ShadowHelper
{
    friend class Decoration;
public:
    enum State {
        Active,
        Inactive,
    };

    static ShadowHelper *globalInstance();

    QSharedPointer<KDecoration2::DecorationShadow> getShadow(State state,
                                                             int shadow_border,                     //阴影边框大小：30小边框、100中边框、200大边框
                                                             qreal darkness,                        //阴影颜色深度：1.0深、1.5很深、2.0超深
                                                             int borderRadiusTopLeft = 0,
                                                             int borderRadiusTopRight = 0,
                                                             int borderRadiusBottomLeft = 0,
                                                             int borderRadiusBottomRight = 0);

private:
    ShadowHelper();
    QPixmap getShadowPixmap(State state,
                            int shadow_border,
                            qreal darkness,
                            int borderRadiusTopLeft = 0,
                            int borderRadiusTopRight = 0,
                            int borderRadiusBottomLeft = 0,
                            int borderRadiusBottomRight = 0);

    QMap<QList<int>, QSharedPointer<KDecoration2::DecorationShadow>> m_inactiveShadowsCache;
    QMap<QList<int>, QSharedPointer<KDecoration2::DecorationShadow>> m_activeShadowsCache;
};

}

#endif // SHADOWHELPER_H
