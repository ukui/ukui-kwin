#include "shadow-helper.h"

using namespace UKUI;

static ShadowHelper *global_instance = nullptr;

ShadowHelper *ShadowHelper::globalInstance()
{
    if (!global_instance)
        global_instance = new ShadowHelper;
    return global_instance;
}

QSharedPointer<KDecoration2::DecorationShadow> ShadowHelper::getShadow(ShadowHelper::State state)
{
    if (auto shadow = m_shadows.value(state)) {
        return shadow;
    }
    auto shadow = QSharedPointer<KDecoration2::DecorationShadow>::create();
    auto pix = this->getShadowPixmap(state);
    shadow->setPadding(QMargins(10, 10, 10, 10));
    shadow->setInnerShadowRect(QRect(pix.width()/2, pix.height()/2, 1, 1));
    auto img = pix.toImage();
    shadow->setShadow(img);

    m_shadows.insert(state, shadow);
    return shadow;
}

QPixmap ShadowHelper::getShadowPixmap(ShadowHelper::State state)
{
    QPixmap pix(QSize(100, 100));
    pix.fill(Qt::green);
    return pix;
}

ShadowHelper::ShadowHelper()
{

}
