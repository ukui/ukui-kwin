#ifndef UKUIDECORATION_H
#define UKUIDECORATION_H

#include <KDecoration2/Decoration>
#include <KPluginFactory>

namespace KDecoration2 {
class DecorationButtonGroup;
}

namespace UKUI {

class Decoration : public KDecoration2::Decoration
{
    Q_OBJECT
public:
    explicit Decoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    void init();

    void paint(QPainter *painter, const QRect &repaintRegion) override;

public slots:
    void updateButtonsGeomerty();

private:
    KDecoration2::DecorationButtonGroup *m_buttons = nullptr;
};

}

#endif // UKUIDECORATION_H
