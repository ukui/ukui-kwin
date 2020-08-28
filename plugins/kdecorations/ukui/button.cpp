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
        case KDecoration2::DecorationButtonType::Close:
            b->setVisible(true);
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

Button::Button(KDecoration2::DecorationButtonType type, UKUI::Decoration *decoration1, QObject *parent) : KDecoration2::DecorationButton(type, decoration1, parent)
{

}

void Button::paint(QPainter *painter, const QRect &repaintRegion)
{
    painter->save();
    //printf("\nButton::paint, x:%f, y:%f, w:%f, h:%f\n", geometry().x(), geometry().y(), geometry().width(), geometry().height());
    painter->setPen(Qt::NoPen);

    if(isPressed())
    {
        if(KDecoration2::DecorationButtonType::Close == type())
        {
            painter->setBrush(QColor(205, 0, 0));   //关闭按钮底色
        }
        else
        {
            painter->setBrush(QColor(0, 0, 205));   //其他按钮底色
        }
    }
    else if(isHovered())
    {
        if(KDecoration2::DecorationButtonType::Close == type())
        {
            painter->setBrush(QColor(255, 0, 0));   //关闭按钮底色
        }
        else
        {
            painter->setBrush(QColor(0, 0, 255));   //其他按钮底色
        }
    }
    else
    {
        painter->setBrush(backgroundColor());
    }

    painter->drawRoundedRect(geometry(), 3, 3);
    drawIcon(painter);

    painter->restore();
}

void Button::drawIcon( QPainter *painter ) const
{
    qreal Symbol = 1.01;
    painter->setRenderHints(QPainter::Antialiasing);

    painter->translate(geometry().topLeft());
    //printf("\n Button::drawIcon geometry().topLeft().x:%f geometry().topLeft().y:%f geometry().width():%f\n",geometry().topLeft().x(), geometry().topLeft().y(), geometry().width());
    const qreal width(geometry().width());
    painter->scale(width/18, width/18);   //因为下面的线条框架都是按18*18的框架比例画的。

    if(isPressed() || isHovered())
    {
        painter->setPen(backgroundColor());
    }
    else
    {
        painter->setPen(foregroundColor());
    }

    painter->setBrush(Qt::NoBrush);

    switch(type())
    {
        case KDecoration2::DecorationButtonType::Close:
        {
            painter->drawLine( QPointF( 6, 6 ), QPointF( 12, 12 ) );
            painter->drawLine( 12, 6, 6, 12 );
            break;
        }

        case KDecoration2::DecorationButtonType::Maximize:
        {
            if( isChecked() )   //还原
            {
                painter->drawLine( QPointF( 7, 7 ), QPointF( 7, 6 ) );
                painter->drawArc(7, 5, 2, 2, 90 * 16, 90 * 16);
                painter->drawLine( QPointF( 8, 5 ), QPointF( 11, 5 ) );
                painter->drawArc(10, 5, 2, 2, 0, 90 * 16);
                painter->drawLine( QPointF( 12, 6 ), QPointF( 12, 9 ) );
                painter->drawArc(10, 8, 2, 2, 270 * 16, 90 * 16);
                painter->drawLine( QPointF( 11, 10 ), QPointF( 10, 10 ) );
                painter->drawRoundedRect(QRect(5, 7, 5, 5), 1, 1);
            } else {            //最大化
                painter->drawRoundedRect(QRect(6, 6, 6, 6), 1, 1);
            }
            break;
        }

        case KDecoration2::DecorationButtonType::Minimize:
        {
            painter->drawLine(QPointF(6, 9), QPointF(12, 9));
            break;
        }

        case KDecoration2::DecorationButtonType::OnAllDesktops:
        {
            painter->setPen(Qt::NoPen);
            painter->setBrush(foregroundColor());

            if(isChecked())
            {
                // outer ring
                painter->drawEllipse(QRectF(3, 3, 12, 12));

                // center dot
                QColor backgroundColor(this->backgroundColor());
                auto d = qobject_cast<Decoration*>(decoration());
                if(!backgroundColor.isValid() && d) backgroundColor = d->titleBarColor();

                if(backgroundColor.isValid())
                {
                    painter->setBrush(backgroundColor);
                    painter->drawEllipse(QRectF(8, 8, 2, 2));
                }

            } else {
                painter->drawPolygon( QVector<QPointF> {
                    QPointF( 6.5, 8.5 ),
                    QPointF( 12, 3 ),
                    QPointF( 15, 6 ),
                    QPointF( 9.5, 11.5 )});

                painter->drawLine(QPointF( 5.5, 7.5), QPointF(10.5, 12.5 ));
                painter->drawLine(QPointF( 12, 6), QPointF(4.5, 13.5 ));
            }
            break;
        }

        case KDecoration2::DecorationButtonType::Shade:
        {
            if (isChecked())
            {
                painter->drawLine(QPointF(4, 5.5), QPointF(14, 5.5));
                painter->drawPolyline(QVector<QPointF>{
                    QPointF(4, 8),
                    QPointF(9, 13),
                    QPointF(14, 8)});

            } else {

                painter->drawLine(QPointF(4, 5.5), QPointF(14, 5.5));
                painter->drawPolyline(QVector<QPointF> {
                    QPointF(4, 13),
                    QPointF(9, 8),
                    QPointF(14, 13)});
            }
            break;
        }

        case KDecoration2::DecorationButtonType::KeepBelow:
        {

            painter->drawPolyline(QVector<QPointF>{
                QPointF(4, 5),
                QPointF(9, 10),
                QPointF(14, 5)});

            painter->drawPolyline(QVector<QPointF>{
                QPointF(4, 9),
                QPointF(9, 14),
                QPointF(14, 9)});
            break;
        }

        case KDecoration2::DecorationButtonType::KeepAbove:
        {
            painter->drawPolyline(QVector<QPointF>{
                QPointF(4, 9),
                QPointF(9, 4),
                QPointF(14, 9)});

            painter->drawPolyline(QVector<QPointF>{
                QPointF(4, 13),
                QPointF(9, 8),
                QPointF(14, 13)});
            break;
        }

        case KDecoration2::DecorationButtonType::ApplicationMenu:
        {
            painter->drawRect(QRectF(3.5, 4.5, 11, 1));
            painter->drawRect(QRectF(3.5, 8.5, 11, 1));
            painter->drawRect(QRectF(3.5, 12.5, 11, 1));
            break;
        }

        case KDecoration2::DecorationButtonType::ContextHelp:
        {
            QPainterPath path;
            path.moveTo(5, 6);
            path.arcTo(QRectF(5, 3.5, 8, 5), 180, -180);
            path.cubicTo(QPointF(12.5, 9.5), QPointF( 9, 7.5 ), QPointF( 9, 11.5 ));
            painter->drawPath(path);
            painter->drawRect(QRectF( 9, 15, 0.5, 0.5));
            break;
        }
        default: break;
    }

}

QColor Button::foregroundColor() const
{
    auto d = qobject_cast<Decoration*>(decoration());
    return d->fontColor();
}

QColor Button::backgroundColor() const
{
    auto d = qobject_cast<Decoration*>(decoration());
    return d->frameColor();
}


