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

#ifndef BUTTON_H
#define BUTTON_H

#include <QObject>

#include "ukui-decoration.h"
#include <KDecoration2/DecorationButton>

namespace UKUI {

    class Button : public KDecoration2::DecorationButton
    {
        Q_OBJECT
    public:
        explicit Button(QObject *parent = nullptr, const QVariantList &args = QVariantList());
        explicit Button(KDecoration2::DecorationButtonType type, UKUI::Decoration *decoration, QObject *parent);

        ~Button();

        //* button creation
        static Button *create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent);

        //* render
        void paint(QPainter *painter, const QRect &repaintRegion) override;
    };
}

#endif // BUTTON_H
