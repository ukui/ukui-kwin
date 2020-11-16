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

#include "ukui-decoration.h"
#include "button.h"

#include <QtDBus>       //必须放xatom-helper.h前面

#include "xatom-helper.h"
#include "breezeboxshadowrenderer.h"

#include <QVariant>
#include <QPainter>
#include <QMessageLogger>
#include <QSettings>
#include <QStandardPaths>

#include <KPluginFactory>
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationButtonGroup>

#include <KConfigGroup>
#include <KSharedConfig>

#define CUSOR_BORDER  10                //边框伸展光标范围
#define Frame_TopRadius 6               //窗体顶部圆角
#define Frame_BottomRadius 3            //窗体底部圆角

#define Font_Size   11                  //字体大小



K_PLUGIN_FACTORY_WITH_JSON(
    UKUIDecotionFactory,
    "kwin-style-ukui.json",
    registerPlugin<UKUI::Decoration>();
)

//static int g_sDecoCount = 0;
//static int g_shadowSizeEnum = 3;
static int g_shadowStrength = 255;
static QColor g_shadowColor = Qt::black;
static QSharedPointer<KDecoration2::DecorationShadow> g_sShadow;

using namespace UKUI;

const CompositeShadowParams g_shadowParams[] = {
    // None
    CompositeShadowParams(),
    // Small
    CompositeShadowParams(
        QPoint(0, 4),
        ShadowParams(QPoint(0, 0), 16, 0.8),      //ShadowParams(QPoint(0, 0), 16, 1),
        ShadowParams(QPoint(0, -2), 8, 0.4)),
    // Medium
    CompositeShadowParams(
        QPoint(0, 8),
        ShadowParams(QPoint(0, 0), 32, 0.9),
        ShadowParams(QPoint(0, -4), 16, 0.3)),
    // Large
    CompositeShadowParams(
        QPoint(0, 12),
        ShadowParams(QPoint(0, 0), 48, 0.8),
        ShadowParams(QPoint(0, -6), 24, 0.2)),
    // Very large
    CompositeShadowParams(
        QPoint(0, 16),
        ShadowParams(QPoint(0, 0), 64, 0.7),
        ShadowParams(QPoint(0, -8), 32, 0.1)),
};

inline CompositeShadowParams lookupShadowParams(int size)
{
    switch (size) {
    case 0:
        return g_shadowParams[0];
    case 1:
        return g_shadowParams[1];
    case 2:
        return g_shadowParams[2];
    case 3:
        return g_shadowParams[3];
    case 4:
        return g_shadowParams[4];
    default:
        // Fallback to the Large size.
        return g_shadowParams[3];
    }
}

using namespace UKUI;

Decoration::Decoration(QObject *parent, const QVariantList &args)
    : KDecoration2::Decoration(parent, args)
{
    int nDpi = 96;
    m_themeId = 0;
    if (false == args.isEmpty())
    {
        const auto map = args.first().toMap();
        auto it = map.constFind(QStringLiteral("dpi"));
        if (it != map.constEnd()) {
            nDpi = it.value().toInt();
        }

        it = map.constFind(QStringLiteral("themeId"));
        if (it != map.constEnd()) {
            m_themeId = it.value().toBool();
            printf("Decoration::Decoration themeId:%d\n",m_themeId);
        }
    }
    int nScaler = qRound(nDpi / 96.0f);
    m_borderLeft = nScaler * 1;
    m_borderTop = nScaler * 38;
    m_borderRight = nScaler * 1;
    m_borderBottom = nScaler * 1;

    m_buttonWidth = nScaler * 30;
    m_buttonHeight = nScaler * 30;

    m_leftButtonWidth = nScaler * 24;
    m_leftButtonHeight = nScaler * 24;

    m_ButtonMarginTop = nScaler * 4;

    m_buttonSpacing = nScaler * 4;

    m_leftButtons = nullptr;
    m_rightButtons = nullptr;

}

void Decoration::init()
{
    bool isDecoBorderOnly = XAtomHelper::isWindowDecorateBorderOnly(client().data()->windowId());   //是否是仅修饰边框    
    if (!isDecoBorderOnly) {
        QDBusConnection::sessionBus().connect(QString(),
                                              QStringLiteral("/KGlobalSettings"),
                                              QStringLiteral("org.kde.KGlobalSettings"),
                                              QStringLiteral("slotThemeChange"),
                                              this, SLOT(themeUpdate(int)));

        themeUpdate(m_themeId);

        calculateBorders();
        //button
        m_leftButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Left, this, &UKUI::Button::create);
        m_leftButtons->setSpacing(m_buttonSpacing);
        printf("Decoration::init m_leftButtons size:%d\n",m_leftButtons->buttons().size());
        m_nleftButtonCout = 0;
        for (const QPointer<KDecoration2::DecorationButton>& button : m_leftButtons->buttons())
        {
            button.data()->setGeometry(QRectF(QPointF(0, 0), QSizeF(m_leftButtonWidth, m_leftButtonHeight)));
            if(false == button.data()->isVisible())
            {
                continue;
            }

            if(KDecoration2::DecorationButtonType::Menu == button.data()->type())
            {
                m_nleftButtonCout++;
            }
        }

        m_rightButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Right, this, &UKUI::Button::create);
        m_rightButtons->setSpacing(m_buttonSpacing);

        m_nrightButtonCout = 0;
        for (const QPointer<KDecoration2::DecorationButton>& button : m_rightButtons->buttons())
        {
            button.data()->setGeometry(QRectF(QPointF(0, 0), QSizeF(m_buttonWidth, m_buttonHeight)));
            if(false == button.data()->isVisible())
            {
                continue;
            }

            if(KDecoration2::DecorationButtonType::Minimize == button.data()->type())
            {
                m_nrightButtonCout++;
            }
            if(KDecoration2::DecorationButtonType::Maximize == button.data()->type())
            {
                m_nrightButtonCout++;
            }
            if(KDecoration2::DecorationButtonType::Close == button.data()->type())
            {
                m_nrightButtonCout++;
            }
        }

        updateButtonsGeometry();
        connect(settings().data(), &KDecoration2::DecorationSettings::decorationButtonsRightChanged, this, &UKUI::Decoration::updateButtonsGeometry);
        connect(client().data(), &KDecoration2::DecoratedClient::sizeChanged, this, &UKUI::Decoration::updateButtonsGeometry);

        connect(client().data(), &KDecoration2::DecoratedClient::paletteChanged, this, static_cast<void (Decoration::*)()>(&Decoration::update));
        //connect(client().data(), &KDecoration2::DecoratedClient::paletteChanged, this, static_cast<void (Decoration::*)()>(&Decoration::themeUpdate));
        connect(client().data(), &KDecoration2::DecoratedClient::activeChanged, this, static_cast<void (Decoration::*)()>(&Decoration::update));
        //connect(client().data(), &KDecoration2::DecoratedClient::activeChanged, this, &Decoration::updateShadow);
        connect(client().data(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::calculateBorders);
        connect(client().data(), &KDecoration2::DecoratedClient::captionChanged, this,
            [this]()
            {
                update();   //更新标题栏标题内容, update(titleBar())这有时不能即时更新标题栏内容
            }
        );

        connect(client().data(), &KDecoration2::DecoratedClient::widthChanged, this, &Decoration::updateTitleBar);
        connect(client().data(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::updateTitleBar);
        updateTitleBar();
    } else {
        setBorders(QMargins(0, 0, 0, 0));
        setResizeOnlyBorders(QMargins(CUSOR_BORDER, CUSOR_BORDER, CUSOR_BORDER, CUSOR_BORDER));
        //connect(client().data(), &KDecoration2::DecoratedClient::activeChanged, this, &Decoration::updateShadow);
    }

    updateShadow();
    update();
}

void Decoration::updateShadow()
{
    if(!g_sShadow){
        const CompositeShadowParams params = lookupShadowParams(1);
        if (params.isNone()) {
            g_sShadow.clear();
            setShadow(g_sShadow);
        }

        auto withOpacity = [](const QColor &color, qreal opacity)->QColor {
            QColor c(color);
            c.setAlphaF(opacity);
            return c;
        };

        const QSize boxSize = BoxShadowRenderer::calculateMinimumBoxSize(params.shadow1.radius)
            .expandedTo(BoxShadowRenderer::calculateMinimumBoxSize(params.shadow2.radius));

        BoxShadowRenderer shadowRenderer;
        shadowRenderer.setBorderRadius(Frame_TopRadius + 0.5);
        shadowRenderer.setBoxSize(boxSize);
        shadowRenderer.setDevicePixelRatio(1.0); // TODO: Create HiDPI shadows?

        const qreal strength = static_cast<qreal>(g_shadowStrength) / 255.0;
        shadowRenderer.addShadow(params.shadow1.offset, params.shadow1.radius, withOpacity(g_shadowColor, params.shadow1.opacity * strength));
        shadowRenderer.addShadow(params.shadow2.offset, params.shadow2.radius, withOpacity(g_shadowColor, params.shadow2.opacity * strength));

        QImage shadowTexture = shadowRenderer.render();

        QPainter painter(&shadowTexture);
        painter.setRenderHint(QPainter::Antialiasing);

        const QRect outerRect = shadowTexture.rect();

        QRect boxRect(QPoint(0, 0), boxSize);
        boxRect.moveCenter(outerRect.center());

        // Mask out inner rect.
        const QMargins padding = QMargins(
            boxRect.left() - outerRect.left() - 3 - params.offset.x(),
            boxRect.top() - outerRect.top() - 3 - params.offset.y(),
            outerRect.right() - boxRect.right() - 3 + params.offset.x(),
            outerRect.bottom() - boxRect.bottom() - 3 + params.offset.y());
        const QRect innerRect = outerRect - padding;

        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);
        painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        painter.drawRoundedRect(innerRect, Frame_TopRadius + 0.5, Frame_TopRadius + 0.5);

        // Draw outline.
        painter.setPen(withOpacity(g_shadowColor, 0.2 * strength));
        painter.setBrush(Qt::NoBrush);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawRoundedRect(innerRect, Frame_TopRadius - 0.5, Frame_TopRadius - 0.5);

        painter.end();

        g_sShadow = QSharedPointer<KDecoration2::DecorationShadow>::create();
        g_sShadow->setPadding(padding);
        g_sShadow->setInnerShadowRect(QRect(outerRect.center(), QSize(1, 1)));
        g_sShadow->setShadow(shadowTexture);
    }

    setShadow(g_sShadow);
    update();
}

void Decoration::updateTitleBar()
{
    bool isDecoBorderOnly = XAtomHelper::isWindowDecorateBorderOnly(client().data()->windowId());
    if (isDecoBorderOnly)
    {
        return;
    }
    auto c = client().data();
    const int x = (m_ButtonMarginTop + m_buttonSpacing) * 2 + m_nleftButtonCout * m_leftButtonWidth;
    const int width =  c->width() + (c->isMaximized() ? 0 : (m_borderLeft + m_borderRight))
            - (m_ButtonMarginTop + m_buttonSpacing) * 2 - m_nleftButtonCout * m_leftButtonWidth         //减去左侧按钮空间
            - m_nrightButtonCout * (m_buttonWidth + m_buttonSpacing);                                   //减去右侧按钮空间
    setTitleBar(QRect(x, 0, width, borderTop()));
}

void Decoration::calculateBorders()
{    
    bool maximized = client().data()->isMaximized();
    if(true == maximized)
    {
        setBorders(QMargins(0, m_borderTop, 0, 0));     //真正的边框尺寸
        setResizeOnlyBorders(QMargins(0, 0, 0, 0));     //边框伸展光标范围
    }
    else
    {
        setBorders(QMargins(m_borderLeft, m_borderTop, m_borderRight, m_borderBottom));                     //真正的边框尺寸
        setResizeOnlyBorders(QMargins(CUSOR_BORDER, CUSOR_BORDER, CUSOR_BORDER, CUSOR_BORDER));             //边框伸展光标范围
    }
}

void Decoration::themeUpdate(int themeId)
{
    m_themeId = themeId;
    printf("Decoration::themeUpdate themeId:%d\n",m_themeId);
    if(1 == m_themeId)
    {
        m_frameColor = QColor(31, 32, 34);
        m_fontActiveColor= QColor(207, 207, 207);
        m_fontInactiveColor= QColor(105, 105, 105);

        //修改标题栏右键菜单颜色
        auto config = KSharedConfig::openConfig(QStringLiteral("kdeglobals"), KConfig::SimpleConfig);
        KConfigGroup viewConfig(config, QStringLiteral("Colors:View"));
        viewConfig.writeEntry("BackgroundAlternate", QColor(49,54,59));
        viewConfig.writeEntry("BackgroundNormal", QColor(35,38,41));
        viewConfig.writeEntry("DecorationHover", QColor(61,174,233));
        viewConfig.writeEntry("ForegroundInactive", QColor(189,195,199));
        viewConfig.writeEntry("ForegroundNormal", QColor(239,240,241));

        KConfigGroup windowConfig(config, QStringLiteral("Colors:Window"));
        windowConfig.writeEntry("BackgroundAlternate", QColor(77,77,77));
        windowConfig.writeEntry("BackgroundNormal", QColor(49,54,59));
        windowConfig.writeEntry("DecorationHover", QColor(61,174,233));
        windowConfig.writeEntry("ForegroundInactive", QColor(189,195,199));
        windowConfig.writeEntry("ForegroundNormal", QColor(239,240,241));
        config->sync();

    }
    else
    {
        m_frameColor = QColor(255, 255, 255);
        m_fontActiveColor= QColor(0, 0, 0);
        m_fontInactiveColor= QColor(105, 105, 105);

        auto config = KSharedConfig::openConfig(QStringLiteral("kdeglobals"), KConfig::SimpleConfig);
        KConfigGroup viewConfig(config, QStringLiteral("Colors:View"));
        viewConfig.writeEntry("BackgroundAlternate", QColor(239,240,241));
        viewConfig.writeEntry("BackgroundNormal", QColor(252,252,252));
        viewConfig.writeEntry("DecorationHover", QColor(147,206,233));
        viewConfig.writeEntry("ForegroundInactive", QColor(127,140,141));
        viewConfig.writeEntry("ForegroundNormal", QColor(35,38,39));

        KConfigGroup windowConfig(config, QStringLiteral("Colors:Window"));
        windowConfig.writeEntry("BackgroundAlternate", QColor(189,195,199));
        windowConfig.writeEntry("BackgroundNormal", QColor(239,240,241));
        windowConfig.writeEntry("DecorationHover", QColor(147,206,233));
        windowConfig.writeEntry("ForegroundInactive", QColor(127,140,141));
        windowConfig.writeEntry("ForegroundNormal", QColor(35,38,39));
        config->sync();
    }

    update();
}

void Decoration::paint(QPainter *painter, const QRect &repaintRegion)
{
    if (!repaintRegion.isEmpty()){
        painter->setClipRect(repaintRegion);
    }

    auto c = client().data();

    //总体框架刷颜色
    painter->save();
    if(c->isMaximized())
    {
        auto rect = QRect(QPoint(0, 0), this->client().data()->size());
        painter->fillRect(rect, frameColor());
    }
    else
    {
        auto rect = QRect(0, 0, (c->size().width() + m_borderLeft + m_borderRight), (c->size().height() + m_borderTop + m_borderBottom));
        painter->setPen(Qt::NoPen);
        painter->setBrush(frameColor());
        painter->drawRoundedRect(rect, Frame_TopRadius, Frame_TopRadius);
        auto rectLeftBottom = QRect(0, rect.height() - Frame_TopRadius * 2, Frame_TopRadius * 2, Frame_TopRadius * 2);
        painter->drawRoundedRect(rectLeftBottom, Frame_BottomRadius, Frame_BottomRadius);   //左下角补角
        auto rectRightBottom = QRect(rect.width() - Frame_TopRadius * 2, rect.height() - Frame_TopRadius * 2, Frame_TopRadius * 2, Frame_TopRadius * 2);
        painter->drawRoundedRect(rectRightBottom, Frame_BottomRadius, Frame_BottomRadius);
    }
    painter->restore();

    //写标题
    QFont font;
    font.setPointSize(Font_Size);         //setPointSize可以根据dpi自动调整，所以m_nFont不需要乘缩放系数，而setPixelSize会写死
    //font.setFamily();
    painter->setFont(font);
    painter->setPen(fontColor());

    const auto cR = qMakePair(titleBar(), Qt::AlignVCenter | Qt::AlignLeft);
    const QString caption = painter->fontMetrics().elidedText(c->caption(), Qt::ElideMiddle, cR.first.width());
    painter->drawText(cR.first, cR.second | Qt::TextSingleLine, caption);

    //按钮组刷颜色
    m_leftButtons->paint(painter, repaintRegion);
    m_rightButtons->paint(painter, repaintRegion);
}

void Decoration::updateButtonsGeometry()
{
    auto c = client().data();

    m_leftButtons->setPos(QPoint(m_ButtonMarginTop + m_buttonSpacing, m_ButtonMarginTop + m_buttonSpacing));

    //由于上边檐是m_ButtonMarginTop，右侧有m_nrightButtonCout个按钮和按钮间隙
    auto posX = c->width() + (c->isMaximized() ? 0 : (m_borderLeft + m_borderRight)) - m_nrightButtonCout * (m_buttonWidth +  m_buttonSpacing);
    m_rightButtons->setPos(QPoint(posX, m_ButtonMarginTop));

    update();
}

QColor Decoration::fontColor()const
{
    auto c = client().data();
    if(c->isActive())
    {
        return m_fontActiveColor;
    }
    else
    {
        return m_fontInactiveColor;
    }
}

QColor Decoration::frameColor() const
{
    return m_frameColor;
}


#include <ukui-decoration.moc>
