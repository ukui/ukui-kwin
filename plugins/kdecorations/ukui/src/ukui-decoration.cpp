#include "ukui-decoration.h"
#include "button.h"
#include "shadow-helper.h"

#include "xatom-helper.h"

#include <QVariant>
#include <KPluginFactory>

#include <QPainter>

#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationButtonGroup>
#include <KDecoration2/DecorationButton>

//using namespace UKUIDecoration;

K_PLUGIN_FACTORY_WITH_JSON(
    UKUIDecotionFactory,
    "kwin-style-ukui.json",
    registerPlugin<UKUI::Decoration>();
)

using namespace UKUI;

Decoration::Decoration(QObject *parent, const QVariantList &args)
    : KDecoration2::Decoration(parent, args)
{
}

void Decoration::init()
{
    bool isDecoBorderOnly = XAtomHelper::isWindowDecorateBorderOnly(client().data()->windowId());
    if (!isDecoBorderOnly) {
        setBorders(QMargins(1, 30, 1, 1));
        setResizeOnlyBorders(QMargins(10, 10, 10, 10));

        //title bar
        auto s = settings();
        const bool maximized = client().data()->isMaximized();
        const int width =  maximized ? client().data()->width() : client().data()->width() - 2*s->largeSpacing()*2;
        const int height = maximized ? borderTop() : borderTop() - s->smallSpacing()*2;
        const int x = maximized ? 0 : s->largeSpacing()*2;
        const int y = maximized ? 0 : s->smallSpacing()*2;
        setTitleBar(QRect(x, y, width, height));

        //button
        m_buttons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Right, this, &UKUI::Button::create);
        updateButtonsGeomerty();
        connect(settings().data(), &KDecoration2::DecorationSettings::decorationButtonsRightChanged, this, &UKUI::Decoration::updateButtonsGeomerty);
        connect(client().data(), &KDecoration2::DecoratedClient::sizeChanged, this, &UKUI::Decoration::updateButtonsGeomerty);
    } else {
        setBorders(QMargins(0, 0, 0, 0));
        setResizeOnlyBorders(QMargins(10, 10, 10, 10));
    }

    //shadow
    auto shadow = ShadowHelper::globalInstance()->getShadow(ShadowHelper::Active);
    setShadow(shadow);

    update();
}

void Decoration::paint(QPainter *painter, const QRect &repaintRegion)
{
    auto extraSize = QSize(2, 31);
    auto rect = QRect(QPoint(0, 0), client().data()->size() + extraSize);
    painter->fillRect(rect, Qt::red);
    m_buttons->paint(painter, repaintRegion);
}

void Decoration::updateButtonsGeomerty()
{
    auto c = client().data();
    auto posX = c->size().width() - 100;
    m_buttons->setPos(QPoint(posX, 0));
    //m_buttons->setSpacing(100);
    for (const QPointer<KDecoration2::DecorationButton>& button : m_buttons->buttons()) {
        button.data()->setGeometry(QRectF(QPointF(0, 0), QSizeF(32, 32)));
    }

    update();
}

#include <ukui-decoration.moc>
