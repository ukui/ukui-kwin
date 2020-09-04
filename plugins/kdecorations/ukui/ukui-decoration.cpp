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
#include "xatom-helper.h"
#include "shadow-helper.h"

#include <QVariant>
#include <QPainter>
#include <QMessageLogger>
#include <QSettings>
#include <QStandardPaths>

#include <KPluginFactory>
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationButtonGroup>

#define CUSOR_BORDER  10                //边框伸展光标范围
#define SHADOW_BORDER 100               //阴影边框大小：30小边框、100中边框、200大边框
#define ACTIVE_DARKNESS 1.5             //阴影颜色深度：1.0深、1.5很深、2.0超深
#define INACTIVE_DARKNESS 1.0           //阴影颜色深度：1.0深、1.5很深、2.0超深
#define RADIUS 4                        //窗体圆角


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
    if (false == args.isEmpty())
    {
        const auto map = args.first().toMap();
        auto it = map.constFind(QStringLiteral("dpi"));
        if (it != map.constEnd()) {
            nDpi = it.value().toInt();
        }
    }

    m_borderLeft = qRound((nDpi / 96.0f) * 1);
    m_borderTop = qRound((nDpi / 96.0f) * 38);
    m_borderRight = qRound((nDpi / 96.0f) * 1);
    m_borderBottom = qRound((nDpi / 96.0f) * 1);

    m_buttonWidth = qRound((nDpi / 96.0f) * 30);
    m_buttonHeight = qRound((nDpi / 96.0f) * 30);

    m_ButtonMarginTop = qRound((nDpi / 96.0f) * 4);

    m_nFont = 12;

    m_frameColor = QColor(255, 255, 255);
    m_fontActiveColor= QColor(0, 0, 0);
    m_fontInactiveColor= QColor(105, 105, 105);
}

void Decoration::init()
{
    bool isDecoBorderOnly = XAtomHelper::isWindowDecorateBorderOnly(client().data()->windowId());   //是否是仅修饰边框
    //printf("\nbegin init ukui-style now:%d\n",isDecoBorderOnly);
    if (!isDecoBorderOnly) {
        calculateBorders();
        //button
        m_buttons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Right, this, &UKUI::Button::create);
        updateButtonsGeometry();
        connect(settings().data(), &KDecoration2::DecorationSettings::decorationButtonsRightChanged, this, &UKUI::Decoration::updateButtonsGeometry);
        connect(client().data(), &KDecoration2::DecoratedClient::sizeChanged, this, &UKUI::Decoration::updateButtonsGeometry);

        connect(client().data(), &KDecoration2::DecoratedClient::paletteChanged, this, static_cast<void (Decoration::*)()>(&Decoration::update));
        connect(client().data(), &KDecoration2::DecoratedClient::paletteChanged, this, static_cast<void (Decoration::*)()>(&Decoration::themeChanged));
        connect(client().data(), &KDecoration2::DecoratedClient::activeChanged, this, &Decoration::updateShadow);
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

        connect(client().data(), &KDecoration2::DecoratedClient::activeChanged, this, &Decoration::updateShadow);
    }

    updateShadow();
    update();
}

void Decoration::updateShadow(bool bActive)
{
    bool isDecoBorderOnly = XAtomHelper::isWindowDecorateBorderOnly(client().data()->windowId());   //是否是仅修饰边框
    if (!isDecoBorderOnly) {
        if(true == bActive)
        {
            auto shadow = ShadowHelper::globalInstance()->getShadow(ShadowHelper::Active, SHADOW_BORDER, ACTIVE_DARKNESS, RADIUS, RADIUS, RADIUS, RADIUS);
            shadow.data()->setPadding(QMargins(SHADOW_BORDER, SHADOW_BORDER, SHADOW_BORDER, SHADOW_BORDER));
            setShadow(shadow);
        }
        else
        {
            auto shadow = ShadowHelper::globalInstance()->getShadow(ShadowHelper::Inactive, SHADOW_BORDER, INACTIVE_DARKNESS, RADIUS, RADIUS, RADIUS, RADIUS);
            shadow.data()->setPadding(QMargins(SHADOW_BORDER, SHADOW_BORDER, SHADOW_BORDER, SHADOW_BORDER));
            setShadow(shadow);
        }
    }
    else
    {
        auto borderRadius = XAtomHelper::getInstance()->getWindowBorderRadius(client().data()->windowId());

        if(true == bActive)
        {
            auto shadow = ShadowHelper::globalInstance()->getShadow(ShadowHelper::Active,
                                                                    SHADOW_BORDER,
                                                                    ACTIVE_DARKNESS,
                                                                    borderRadius.topLeft,
                                                                    borderRadius.topRight,
                                                                    borderRadius.bottomLeft,
                                                                    borderRadius.bottomRight);
            setShadow(shadow);
        }
        else
        {
            auto shadow = ShadowHelper::globalInstance()->getShadow(ShadowHelper::Inactive,
                                                                    SHADOW_BORDER,
                                                                    INACTIVE_DARKNESS,
                                                                    borderRadius.topLeft,
                                                                    borderRadius.topRight,
                                                                    borderRadius.bottomLeft,
                                                                    borderRadius.bottomRight);
            setShadow(shadow);
        }
    }
}

void Decoration::updateTitleBar()
{
    bool isDecoBorderOnly = XAtomHelper::isWindowDecorateBorderOnly(client().data()->windowId());
    if (isDecoBorderOnly)
    {
        return;
    }
    auto c = client().data();
    const int x = c->isMaximized() ? 0 : m_borderLeft;  //当不是最大化时，带边框，x从左边框起计算
    const int width =  c->width() - 3 * m_buttonWidth - m_ButtonMarginTop + (c->isMaximized() ? 0 : m_borderRight);  //当不是最大化有边框时，标题宽度可向右偏移右边框宽度
    setTitleBar(QRect(x, 0, width, borderTop()));
}

void Decoration::calculateBorders()
{    
    const bool maximized = client().data()->isMaximized();

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

void Decoration::themeChanged()
{
    QString configFileName = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config/kdeglobals";
    //printf("\nDecoration::themeChanged:%s\n", QStandardPaths::writableLocation(QStandardPaths::HomeLocation).toStdString().c_str());
    QSettings* themeSettings = new QSettings(configFileName, QSettings::IniFormat);

    themeSettings->beginGroup("Theme");
    int themeId = themeSettings->value("Style").toInt();
    themeSettings->endGroup();
    delete themeSettings;

    if(1 == themeId)
    {
        m_frameColor = QColor(31, 32, 34);
        m_fontActiveColor= QColor(207, 207, 207);
        m_fontInactiveColor= QColor(105, 105, 105);
    }
    else
    {
        m_frameColor = QColor(255, 255, 255);
        m_fontActiveColor= QColor(0, 0, 0);
        m_fontInactiveColor= QColor(105, 105, 105);
    }
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
        painter->drawRoundedRect(rect, RADIUS, RADIUS);
    }
    painter->restore();

    //写标题
    QFont font;
    font.setPointSize(m_nFont);         //setPointSize可以根据dpi自动调整，所以m_nFont不需要乘缩放系数，而setPixelSize会写死
    //font.setFamily();
    painter->setFont(font);
    painter->setPen(fontColor());

    const auto cR = captionRect();
    const QString caption = painter->fontMetrics().elidedText(c->caption(), Qt::ElideMiddle, cR.first.width());
    painter->drawText(cR.first, cR.second | Qt::TextSingleLine, caption);

    //按钮组刷颜色
    m_buttons->paint(painter, repaintRegion);
}

void Decoration::updateButtonsGeometry()
{
    m_buttons->setSpacing(0);
    for (const QPointer<KDecoration2::DecorationButton>& button : m_buttons->buttons()) {
        button.data()->setGeometry(QRectF(QPointF(0, 0), QSizeF(m_buttonWidth, m_buttonHeight)));
    }

    auto c = client().data();
    //由于上边檐是m_ButtonMarginTop，为了美观按钮组右边空白也应该是m_ButtonMarginTop这么多，但是当窗体不是最大化时，本身带有边框，故需要向右偏移m_borderLeft和m_borderRight
    auto posX = c->width() - 3 * m_buttonWidth - m_ButtonMarginTop + (c->isMaximized() ? 0 : (m_borderLeft + m_borderRight));
    m_buttons->setPos(QPoint(posX, m_ButtonMarginTop));

    update();
}

QPair<QRect,Qt::Alignment> Decoration::captionRect() const
{
    auto c = client().data();
    const int x = c->isMaximized() ? 0 : m_borderLeft;
    const int width =  c->width() - 3 * m_buttonWidth - m_ButtonMarginTop + (c->isMaximized() ? 0 : m_borderRight);  //当不是最大化有边框时，标题宽度可向右偏移右边框宽度

    const QRect maxRect(x, 0, width, borderTop());
    return qMakePair(maxRect, Qt::AlignVCenter | Qt::AlignHCenter);
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

QColor Decoration::titleBarColor() const
{
    auto c = client().data();
    return c->color(KDecoration2::ColorGroup::Inactive, KDecoration2::ColorRole::TitleBar);
}


#include <ukui-decoration.moc>
