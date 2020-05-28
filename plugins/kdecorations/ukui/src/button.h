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

    //* button creation
    static Button *create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent);

    //* render
    void paint(QPainter *painter, const QRect &repaintRegion) override;
};

}

#endif // BUTTON_H
