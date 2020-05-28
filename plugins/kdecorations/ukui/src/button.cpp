#include "button.h"
#include <QPainter>

using namespace UKUI;

Button::Button(QObject *parent, const QVariantList &args) : KDecoration2::DecorationButton(args.at(0).value<KDecoration2::DecorationButtonType>(), args.at(1).value<Decoration*>(), parent)
{

}

Button::Button(KDecoration2::DecorationButtonType type, UKUI::Decoration *decoration, QObject *parent) : KDecoration2::DecorationButton(type, decoration, parent)
{
    const int height = 32;
    //setGeometry(QRect(0, 0, height, height));
    //setIconSize(QSize( height, height ));
}

Button *Button::create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
{
    auto d = static_cast<UKUI::Decoration *>(decoration);
    auto b = new Button(type, d, parent);
    switch (type) {
    case KDecoration2::DecorationButtonType::Close:
        b->setVisible(true);
        break;
    default:
        break;
    }

    return b;
}

void Button::paint(QPainter *painter, const QRect &repaintRegion)
{
    auto rect = QRectF(QPoint(0, 0), QSize(32, 32));
    painter->fillRect(geometry().adjusted(2, 2, -2, -2), Qt::blue);
    //painter->fillRect(-9999, -9999, 9999, 9999, Qt::blue);
}


