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

#include "button.h"
#include <QPainter>
#include <KDecoration2/DecoratedClient>

using namespace UKUI;

Button *Button::create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
{
    auto d = static_cast<UKUI::Decoration *>(decoration);
    auto b = new Button(type, d, parent);
    switch (type) {
        case KDecoration2::DecorationButtonType::Menu:
            QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::iconChanged, b, [b]() {b->update();});
            break;

        case KDecoration2::DecorationButtonType::OnAllDesktops:
            b->setVisible(false);
            break;

        case KDecoration2::DecorationButtonType::Close:
            break;

        case KDecoration2::DecorationButtonType::Maximize:
            //QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::maximizeableChanged, b, &Button::setVisible );
            break;

        case KDecoration2::DecorationButtonType::Minimize:
            //QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::minimizeableChanged, b, &Button::setVisible );
            break;

        case KDecoration2::DecorationButtonType::ContextHelp:
            b->setVisible(false);

        default:
            break;
    }

    return b;
}

Button::Button(QObject *parent, const QVariantList &args) : KDecoration2::DecorationButton(args.at(0).value<KDecoration2::DecorationButtonType>(), args.at(1).value<Decoration*>(), parent)
{

}

Button::Button(KDecoration2::DecorationButtonType type, UKUI::Decoration *decoration, QObject *parent) : KDecoration2::DecorationButton(type, decoration, parent)
{

}

Button::~Button()
{

}

void Button::paint(QPainter *painter, const QRect &repaintRegion)
{
    if(false == isVisible())
    {
        return;
    }

    if(!decoration())
    {
        return;
    }

    auto d = qobject_cast<Decoration*>(decoration());

    //menu button
    if(type() == KDecoration2::DecorationButtonType::Menu)
    {
        const QRectF iconRect(geometry().topLeft(), geometry().size().toSize());
        decoration()->client().data()->icon().paint(painter, iconRect.toRect());
        return;
    }

    QString strPath;
    switch (type()) {
        case KDecoration2::DecorationButtonType::Minimize:
            if(isPressed())
            {
                if(0 == d->themeId())
                {
                    strPath = ":icon/clicked-minimize-black.svg";
                }
                else
                {
                    strPath = ":icon/clicked-minimize-white.svg";
                }
            }
            else if(isHovered())
            {
                if(0 == d->themeId())
                {
                    strPath = ":icon/hover-minimize-black.svg";
                }
                else
                {
                    strPath = ":icon/hover-minimize-white.svg";
                }
            }
            else
            {
                if(0 == d->themeId())
                {
                    strPath = ":icon/common-minimize-black.svg";
                }
                else
                {
                    strPath = ":icon/common-minimize-white.svg";
                }
            }
            break;

        case KDecoration2::DecorationButtonType::Maximize:
            if(isPressed())
            {
                if(isChecked()) //还原
                {
                    if(0 == d->themeId())
                    {
                        strPath = ":icon/clicked-restore-black.svg";
                    }
                    else
                    {
                        strPath = ":icon/clicked-restore-white.svg";
                    }
                }
                else            //最大化
                {
                    if(0 == d->themeId())
                    {
                        strPath = ":icon/clicked-maximize-black.svg";
                    }
                    else
                    {
                        strPath = ":icon/clicked-maximize-white.svg";
                    }
                }
            }
            else if(isHovered())
            {
                if(isChecked()) //还原
                {
                    if(0 == d->themeId())
                    {
                        strPath = ":icon/hover-restore-black.svg";
                    }
                    else
                    {
                        strPath = ":icon/hover-restore-white.svg";
                    }
                }
                else            //最大化
                {
                    if(0 == d->themeId())
                    {
                        strPath = ":icon/hover-maximize-black.svg";
                    }
                    else
                    {
                        strPath = ":icon/hover-maximize-white.svg";
                    }
                }
            }
            else
            {
                if(isChecked()) //还原
                {
                    if(0 == d->themeId())
                    {
                        strPath = ":icon/common-restore-black.svg";
                    }
                    else
                    {
                        strPath = ":icon/common-restore-white.svg";
                    }
                } else          //最大化
                {
                    if(0 == d->themeId())
                    {
                        strPath = ":icon/common-maximize-black.svg";
                    }
                    else
                    {
                        strPath = ":icon/common-maximize-white.svg";
                    }
                }
            }
            break;

        case KDecoration2::DecorationButtonType::Close:
            if(isPressed())
            {
                strPath = ":icon/clicked-close.svg";
            }
            else if(isHovered())
            {
                strPath = ":icon/hover-close.svg";
            }
            else
            {
                if(0 == d->themeId())
                {
                    strPath = ":icon/common-close-black.svg";
                }
                else
                {
                    strPath = ":icon/common-close-white.svg";
                }
            }
            break;

        default:
            break;
    }

    const QRectF iconRect(geometry().topLeft(), geometry().size().toSize());
    QIcon icon(strPath);
    icon.paint(painter, iconRect.toRect());

    return;
}


