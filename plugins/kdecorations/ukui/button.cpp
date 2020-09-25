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
            b->setVisible(d->client().data()->isCloseable());
            break;

        case KDecoration2::DecorationButtonType::Maximize:
            b->setVisible( d->client().data()->isMaximizeable());
            //QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::maximizeableChanged, b, &Button::setVisible );
            break;

        case KDecoration2::DecorationButtonType::Minimize:
            b->setVisible( d->client().data()->isMinimizeable());
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

Button::Button(KDecoration2::DecorationButtonType type, UKUI::Decoration *decoration1, QObject *parent) : KDecoration2::DecorationButton(type, decoration1, parent)
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

    painter->save();
    //printf("\nButton::paint, x:%f, y:%f, w:%f, h:%f\n", geometry().x(), geometry().y(), geometry().width(), geometry().height());
    painter->setPen(Qt::NoPen);
    switch (type()) {
        case KDecoration2::DecorationButtonType::Close:
            if(isPressed())
            {
                painter->setBrush(QColor(215, 52, 53));   //关闭按钮底色
            }
            else if(isHovered())
            {
                painter->setBrush(QColor(240, 65, 52));   //关闭按钮底色
            }
            else
            {
                painter->setBrush(backgroundColor());
            }
            break;

        case KDecoration2::DecorationButtonType::Maximize:
        case KDecoration2::DecorationButtonType::Minimize:
            if(isPressed())
            {
                painter->setBrush(QColor(50, 87, 202));   //其他按钮底色
            }
            else if(isHovered())
            {
                painter->setBrush(QColor(61, 107, 229));   //其他按钮底色
            }
            else
            {
                painter->setBrush(backgroundColor());
            }
            break;

        default:
            break;
    }

    // menu button
    if(type() == KDecoration2::DecorationButtonType::Menu)
    {
        const QRectF iconRect(geometry().topLeft(), geometry().size().toSize());
        decoration()->client().data()->icon().paint(painter, iconRect.toRect());
    }
    else
    {
        painter->drawRoundedRect(geometry(), 3, 3);
        drawIcon( painter );
    }

    painter->restore();
}

void Button::drawIcon( QPainter *painter ) const
{
    qreal Symbol = 1.01;
    painter->setRenderHints(QPainter::Antialiasing);

    painter->translate(geometry().topLeft());
    //printf("\n Button::drawIcon geometry().topLeft().x:%f geometry().topLeft().y:%f geometry().width():%f\n",geometry().topLeft().x(), geometry().topLeft().y(), geometry().width());
    const qreal width(geometry().width());
    painter->scale(width/30, width/30);   //因为下面的线条框架都是按30*30的框架比例画的。

    QPen pen;
    pen.setWidthF(30/width);     //按钮图标线宽为1个单位
    if(isPressed() || isHovered())
    {
        pen.setColor(backgroundColor());
    }
    else
    {
        pen.setColor(foregroundColor());
    }

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    switch(type())
    {
        case KDecoration2::DecorationButtonType::Close:
        {
            painter->drawLine(QPointF(9, 9), QPointF(21, 21));
            painter->drawLine(QPointF(9, 21), QPointF(21, 9));
            break;
        }

        case KDecoration2::DecorationButtonType::Maximize:
        {
            if(isChecked())   //还原
            {
                painter->drawLine(QPointF(11, 11), QPointF(11, 10));
                painter->drawArc(11, 8, 4, 4, 90 * 16, 90 * 16);

                painter->drawLine(QPointF(13, 8), QPointF(19, 8));
                painter->drawArc(17, 8, 4, 4, 0, 90 * 16);

                painter->drawLine(QPointF(21, 10), QPointF(21, 16));
                painter->drawArc(17, 14, 4, 4, 270 * 16, 90 * 16);

                painter->drawLine(QPointF(19, 18), QPointF(18, 18));

                painter->drawRoundedRect(QRect(8, 11, 10, 10), 2, 2);
            } else
            {                   //最大化
                painter->drawRoundedRect(QRect(9, 9, 12, 12), 2, 2);
            }
            break;
        }

        case KDecoration2::DecorationButtonType::Minimize:
        {
            painter->drawLine(QPointF(10, 14.5), QPointF(20, 14.5));
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


