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

#define CUSOR_BORDER  10                //边框伸展光标范围
#define Frame_FrameRadius 6             //窗体圆角


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
        ShadowParams(QPoint(0, 0), 16, 1),
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
        m_buttons->setSpacing(0);

        m_nButtonCout = 0;
        for (const QPointer<KDecoration2::DecorationButton>& button : m_buttons->buttons())
        {
            button.data()->setGeometry(QRectF(QPointF(0, 0), QSizeF(m_buttonWidth, m_buttonHeight)));
            if(false == button.data()->isVisible())
            {
                continue;
            }

            if(KDecoration2::DecorationButtonType::Minimize == button.data()->type())
            {
                m_nButtonCout++;
            }
            if(KDecoration2::DecorationButtonType::Maximize == button.data()->type())
            {
                m_nButtonCout++;
            }
            if(KDecoration2::DecorationButtonType::Close == button.data()->type())
            {
                m_nButtonCout++;
            }
        }

        updateButtonsGeometry();
        connect(settings().data(), &KDecoration2::DecorationSettings::decorationButtonsRightChanged, this, &UKUI::Decoration::updateButtonsGeometry);
        connect(client().data(), &KDecoration2::DecoratedClient::sizeChanged, this, &UKUI::Decoration::updateButtonsGeometry);

        connect(client().data(), &KDecoration2::DecoratedClient::paletteChanged, this, static_cast<void (Decoration::*)()>(&Decoration::update));
        connect(client().data(), &KDecoration2::DecoratedClient::paletteChanged, this, static_cast<void (Decoration::*)()>(&Decoration::themeChanged));
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
        const CompositeShadowParams params = lookupShadowParams(3);
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
        shadowRenderer.setBorderRadius(Frame_FrameRadius + 0.5);
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
        painter.drawRoundedRect(innerRect, Frame_FrameRadius + 0.5, Frame_FrameRadius + 0.5);

        // Draw outline.
        painter.setPen(withOpacity(g_shadowColor, 0.2 * strength));
        painter.setBrush(Qt::NoBrush);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawRoundedRect(innerRect, Frame_FrameRadius - 0.5, Frame_FrameRadius - 0.5);

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
    const int x = c->isMaximized() ? 0 : m_borderLeft;  //当不是最大化时，带边框，x从左边框起计算
    const int width =  c->width() - m_nButtonCout * m_buttonWidth - m_ButtonMarginTop + (c->isMaximized() ? 0 : m_borderRight);  //当不是最大化有边框时，标题宽度可向右偏移右边框宽度
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
        painter->drawRoundedRect(rect, Frame_FrameRadius, Frame_FrameRadius);
        auto rectLeftBottom = QRect(0, rect.height() - Frame_FrameRadius, Frame_FrameRadius, Frame_FrameRadius);
        painter->drawRoundedRect(rectLeftBottom, 3, 3); //左下角补角
        auto rectRightBottom = QRect(rect.width() - Frame_FrameRadius, rect.height() - Frame_FrameRadius, Frame_FrameRadius, Frame_FrameRadius);
        painter->drawRoundedRect(rectRightBottom, 3, 3);
    }
    painter->restore();

    //写标题
    QFont font;
    font.setPointSize(m_nFont);         //setPointSize可以根据dpi自动调整，所以m_nFont不需要乘缩放系数，而setPixelSize会写死
    //font.setFamily();
    painter->setFont(font);
    painter->setPen(fontColor());

    const auto cR = qMakePair(titleBar(), Qt::AlignVCenter | Qt::AlignHCenter);
    const QString caption = painter->fontMetrics().elidedText(c->caption(), Qt::ElideMiddle, cR.first.width());
    painter->drawText(cR.first, cR.second | Qt::TextSingleLine, caption);

    //按钮组刷颜色
    m_buttons->paint(painter, repaintRegion);
}

void Decoration::updateButtonsGeometry()
{
    auto c = client().data();
    //由于上边檐是m_ButtonMarginTop，为了美观按钮组右边空白也应该是m_ButtonMarginTop这么多，但是当窗体不是最大化时，本身带有边框，故需要向右偏移m_borderLeft和m_borderRight
    auto posX = c->width() - m_nButtonCout * m_buttonWidth - m_ButtonMarginTop + (c->isMaximized() ? 0 : (m_borderLeft + m_borderRight));
    m_buttons->setPos(QPoint(posX, m_ButtonMarginTop));

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

QColor Decoration::titleBarColor() const
{
    auto c = client().data();
    return c->color(KDecoration2::ColorGroup::Inactive, KDecoration2::ColorRole::TitleBar);
}


#include <ukui-decoration.moc>
