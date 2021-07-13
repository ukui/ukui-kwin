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

#include <kwineffects.h>

#include "ukui-decoration.h"
#include "button.h"

#include <QtDBus>       //必须放xatom-helper.h前面
#include "shadow-helper.h"
#include "xatom-helper.h"

#include <QPainter>

#include <KPluginFactory>
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationButtonGroup>

#include <KConfigGroup>
#include <KSharedConfig>


#define FONT_SIZE   11                  //字体大小
#define CUSOR_BORDER  10                //边框伸展光标范围
#define SHADOW_BORDER 30                //阴影边框大小：30小边框、100中边框、200大边框
#define ACTIVE_DARKNESS 0.45            //阴影颜色深度：1.0深、1.5很深、2.0超深
#define RADIUS 6


K_PLUGIN_FACTORY_WITH_JSON(
    UKUIDecotionFactory,
    "kwin-style-ukui.json",
    registerPlugin<UKUI::Decoration>();
)

using namespace UKUI;

Decoration::Decoration(QObject *parent, const QVariantList &args)
    : KDecoration2::Decoration(parent, args)
{
    int nDpi = 96;
    m_themeId = 0;
    m_Font.setPointSize(FONT_SIZE);     //setPointSize可以根据dpi自动调整，所以m_nFont不需要乘缩放系数，而setPixelSize会写死
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
        }

        it = map.constFind(QStringLiteral("systemFontSize"));
        if (it != map.constEnd()) {
            m_Font.setPointSize(it.value().toInt());
        }

        it = map.constFind(QStringLiteral("systemFont"));
        if (it != map.constEnd()) {
            m_Font.setFamily(it.value().toString());
        }
    }
    int nScaler = qRound(nDpi / 96.0f);
    m_borderLeft = nScaler * 0;
    m_borderTop = nScaler * 38;
    m_borderRight = nScaler * 0;
    m_borderBottom = nScaler * 0;

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
    m_shadowRadius = RADIUS;

    //从读取配置文件的方式，判断kwin是否能开启毛玻璃效果，如果不能，则阴影圆角为0.
    auto config = KSharedConfig::openConfig("ukui-kwinrc");
    auto group = KConfigGroup(config, "Compositing");
    if (group.readEntry("Backend") == "XRender" || group.readEntry("OpenGLIsUnsafe") == "true") {
        m_shadowRadius = 0;
    }

    XAtomHelper::getInstance()->setUKUIDecoraiontHint(client().data()->windowId(), true);

    QDBusConnection::sessionBus().connect(QString(),
                                          QStringLiteral("/KGlobalSettings"),
                                          QStringLiteral("org.kde.KGlobalSettings"),
                                          QStringLiteral("slotThemeChange"),
                                          this, SLOT(themeUpdate(int)));
    themeUpdate(m_themeId);

    bool isDecoBorderOnly = XAtomHelper::getInstance()->isWindowDecorateBorderOnly(client().data()->windowId());   //是否是仅修饰边框
    if (!isDecoBorderOnly) {
        calculateBorders();
        //button
        m_leftButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Left, this, &UKUI::Button::create);
        m_leftButtons->setSpacing(m_buttonSpacing);

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

        calculateRightButtonCout();

        connect(settings().data(), &KDecoration2::DecorationSettings::decorationButtonsRightChanged, this, &UKUI::Decoration::updateButtonsGeometry);
        connect(settings().data(), &KDecoration2::DecorationSettings::fontChanged, this, &UKUI::Decoration::updatefont);
        connect(client().data(), &KDecoration2::DecoratedClient::sizeChanged, this, &UKUI::Decoration::updateButtonsGeometry);
        connect(client().data(), &KDecoration2::DecoratedClient::paletteChanged, this, static_cast<void (Decoration::*)()>(&Decoration::update));
        connect(client().data(), &KDecoration2::DecoratedClient::activeChanged, this, static_cast<void (Decoration::*)()>(&Decoration::update));
        connect(client().data(), &KDecoration2::DecoratedClient::maximizeableChanged, this, &Decoration::calculateRightButtonCout); //安装兼容应用全屏后还原，可最大化按钮有改变
        connect(client().data(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::calculateBorders);
        connect(client().data(), &KDecoration2::DecoratedClient::captionChanged, this,
            [this]()
            {
                update();   //更新标题栏标题内容, update(titleBar())这有时不能即时更新标题栏内容
            }
        );

        connect(client().data(), &KDecoration2::DecoratedClient::widthChanged, this, &Decoration::updateTitleBar);
        connect(client().data(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::updateTitleBar);
    } else {
        setBorders(QMargins(0, 0, 0, 0));
        setResizeOnlyBorders(QMargins(CUSOR_BORDER, CUSOR_BORDER, CUSOR_BORDER, CUSOR_BORDER));
        //connect(client().data(), &KDecoration2::DecoratedClient::activeChanged, this, &Decoration::updateShadow);
    }
    connect(client().data(), &KDecoration2::DecoratedClient::sizeChanged, this, [=](){
        auto effectManager = KWin::effects;
        if (!effectManager)
            return;
        for (auto window : effectManager->stackingOrder()) {
            if (!window)
                continue;
            bool sameCaption = false;
            bool sameSize = false;
            if (window->caption() == this->client().data()->caption()) {
                sameCaption = true;
            }
            if (window->geometry().size() == client().data()->decoration().data()->size()) {
                sameSize = true;
            }
            if (sameCaption && sameSize) {
                bool isEdge = client().data()->adjacentScreenEdges() != Qt::Edge();
                if (isEdge) {
                    window->setData(1000, true);
                } else {
                    window->setData(1000, QVariant());
                }
            }
        }
    });
}

void Decoration::updatefont(QFont font)
{
    m_Font = font;
    update();
}

void Decoration::updateShadow()
{
    auto ubr = XAtomHelper::getInstance()->getWindowBorderRadius(client().data()->windowId());
    
    // 控制左上、右上的阴影，ubr不生效时，阴影也应该是0
    if (ubr.topLeft <= 0) {
        ubr.topLeft = m_shadowRadius;
    }
    if (ubr.topRight <= 0) {
        ubr.topRight = m_shadowRadius;
    }
    if (ubr.bottomLeft <= 0) {
        ubr.bottomLeft = m_shadowRadius;
    }
    if (ubr.bottomRight <= 0) {
        ubr.bottomRight = m_shadowRadius;
    }

    ShadowIndex shadowIndex(this->fontColor(), ubr.topLeft, ubr.topRight, ubr.bottomLeft, ubr.bottomRight, ACTIVE_DARKNESS, SHADOW_BORDER);

    auto shadow = ShadowHelper::globalInstance()->getShadow(shadowIndex);
    shadow.data()->setPadding(QMargins(SHADOW_BORDER, SHADOW_BORDER, SHADOW_BORDER, SHADOW_BORDER));
    setShadow(shadow);
}

void Decoration::updateTitleBar()
{
    bool isDecoBorderOnly = XAtomHelper::getInstance()->isWindowDecorateBorderOnly(client().data()->windowId());
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
        //在最大化时需要对setBorders中其中一个参数调整一下大小，否则，兆芯笔记本使用VSCode开源软件最大化时，会出现页面刷新不完全的情况。此处选择将m_borderTop - 1进行调整。
        //后来由于使用新版VSCode不会出现该问题，故又将m_borderTop - 1调整回m_borderTop
        setBorders(QMargins(0, m_borderTop, 0, 0));     //真正的边框尺寸
        setResizeOnlyBorders(QMargins(0, 0, 0, 0));     //边框伸展光标范围
    }
    else
    {
        setBorders(QMargins(m_borderLeft, m_borderTop, m_borderRight, m_borderBottom));                     //真正的边框尺寸
        setResizeOnlyBorders(QMargins(CUSOR_BORDER, CUSOR_BORDER, CUSOR_BORDER, CUSOR_BORDER));             //边框伸展光标范围
    }
}

void Decoration::calculateRightButtonCout()
{
    m_nrightButtonCout = 0;
    for (const QPointer<KDecoration2::DecorationButton>& button : m_rightButtons->buttons())
    {
        if(KDecoration2::DecorationButtonType::Minimize == button.data()->type() &&
          (false == client().data()->isMinimizeable() || false == XAtomHelper::getInstance()->isShowMinimizeButton(client().data()->windowId())))
        {
            button.data()->setVisible(false);
            continue;
        }

        //安卓兼容应用全屏后还原，从可最大化转变为不可最大化，此处增加隐藏按钮设定
        if(KDecoration2::DecorationButtonType::Maximize == button.data()->type())
        {
            if(false == client().data()->isMaximizeable())
            {
                button.data()->setVisible(false);
                continue;
            }
            else
            {
                button.data()->setVisible(true);
            }
        }

        if(KDecoration2::DecorationButtonType::Close == button.data()->type())
        {
            if(false == client().data()->isCloseable())
            {
                button.data()->setVisible(false);
                continue;
            }
            else
            {
                button.data()->setVisible(true);
            }
        }

        button.data()->setGeometry(QRectF(QPointF(0, 0), QSizeF(m_buttonWidth, m_buttonHeight)));

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
    updateTitleBar();
}

void Decoration::themeUpdate(int themeId)
{
    m_themeId = themeId;
    if(1 == m_themeId)
    {
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
    updateShadow();
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
        painter->fillRect(rect, c->palette().color(QPalette::Active, QPalette::Base));
    }
    else if(c->adjacentScreenEdges() != Qt::Edge())
    {
        auto rect = QRect(0, 0, (c->size().width() + m_borderLeft + m_borderRight), (c->size().height() + m_borderTop + m_borderBottom));
        painter->setPen(Qt::NoPen);
        painter->setBrush(c->palette().color(QPalette::Active, QPalette::Base));
        painter->drawRoundedRect(rect, 0, 0);
    }
    else
    {
        auto rect = QRect(0, 0, (c->size().width() + m_borderLeft + m_borderRight), (c->size().height() + m_borderTop + m_borderBottom));
        painter->setPen(Qt::NoPen);
        painter->setBrush(c->palette().color(QPalette::Active, QPalette::Base));
        // 控制左上、右上的阴影
        painter->drawRoundedRect(rect, m_shadowRadius, m_shadowRadius);

        auto rectLeftBottom = QRect(0, rect.height() - m_shadowRadius * 2, m_shadowRadius * 2, m_shadowRadius * 2);
        painter->drawRoundedRect(rectLeftBottom, 0, 0);   //左下角补角
        auto rectRightBottom = QRect(rect.width() - m_shadowRadius * 2, rect.height() - m_shadowRadius * 2, m_shadowRadius * 2, m_shadowRadius * 2);
        painter->drawRoundedRect(rectRightBottom, 0, 0);

    }
    painter->restore();

    //写标题
    painter->setFont(m_Font);
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


#include <ukui-decoration.moc>
