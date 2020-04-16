/********************************************************************
 UKUI-KWin - the UKUI3.0 window manager
 This file is part of the UKUI project
 The ukui-kwin is forked from kwin

Copyright (C) 2014-2020 kylinos.cn

 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2014 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "settings.h"
#include "gsettingfont.h"
// KWin
#include "decorationbridge.h"
#include "composite.h"
#include "virtualdesktops.h"
#include "workspace.h"
#include "appmenu.h"

#include <config-ukui-kwin.h>
#include <KDecoration2/DecorationSettings>
#include <KConfigGroup>
#include <QFontDatabase>
#include <QtWidgets>

#define DEFAULT_FONTSIZE 15

namespace KWin
{
namespace Decoration
{
SettingsImpl::SettingsImpl(KDecoration2::DecorationSettings *parent)
    : QObject()
    , DecorationSettingsPrivate(parent)
    , m_borderSize(KDecoration2::BorderSize::Normal)
{
    m_nScaleFactor = 1;                       //放大系数
    QDesktopWidget* m_pDeskWgt = QApplication::desktop();
    QRect screenRect = m_pDeskWgt->screenGeometry();
    if(screenRect.height() >= 2000)
    {
        m_nScaleFactor = 2;
    }

    readSettings();

    auto c = connect(Compositor::self(), &Compositor::compositingToggled,
            parent, &KDecoration2::DecorationSettings::alphaChannelSupportedChanged);
    connect(VirtualDesktopManager::self(), &VirtualDesktopManager::countChanged, this,
        [parent](uint previous, uint current) {
            if (previous != 1 && current != 1) {
                return;
            }
            emit parent->onAllDesktopsAvailableChanged(current > 1);
        }
    );
    // prevent changes in Decoration due to Compositor being destroyed
    connect(Compositor::self(), &Compositor::aboutToDestroy, this,
        [this, c] {
            disconnect(c);
        }
    );
    connect(Workspace::self(), &Workspace::configChanged, this, &SettingsImpl::readSettings);
    connect(DecorationBridge::self(), &DecorationBridge::metaDataLoaded, this, &SettingsImpl::readSettings);

    const QByteArray schema("org.ukui.style");
    const QByteArray path("/org/ukui/style/");
    m_pSettings = new QGSettings(schema, path, this);
    connect(m_pSettings, SIGNAL(changed(const QString&)), this, SLOT(onGSettingChangedSlot()));

    if (true == m_pSettings->keys().contains("systemFontSize"))
    {
        QString strAppName = m_pSettings->get("system-font-size").toString();
        fputs("SettingsImpl::SettingsImpl,  获取到QGSettings值\n", stderr);
        puts(strAppName.toLatin1().data());
        puts(strAppName.toStdString().c_str());
    }

    GSettingFont* p = new GSettingFont;
    connect(p, SIGNAL(Sig_fontChanged(int)), this, SLOT(onFontChanged(int)));

    QDBusConnection connection = QDBusConnection::sessionBus();
    if(connection.registerService("com.ukui.kwin"))
    {
        connection.registerObject("/Ukui/kwin", p, QDBusConnection::ExportAllSlots);
    }
}

SettingsImpl::~SettingsImpl() = default;

bool SettingsImpl::isAlphaChannelSupported() const
{
    return Compositor::self()->compositing();
}

bool SettingsImpl::isOnAllDesktopsAvailable() const
{
    return VirtualDesktopManager::self()->count() > 1;
}

bool SettingsImpl::isCloseOnDoubleClickOnMenu() const
{
    return m_closeDoubleClickMenu;
}

static QHash<KDecoration2::DecorationButtonType, QChar> s_buttonNames;
static void initButtons()
{
    if (!s_buttonNames.isEmpty()) {
        return;
    }
    s_buttonNames[KDecoration2::DecorationButtonType::Menu]            = QChar('M');
    s_buttonNames[KDecoration2::DecorationButtonType::ApplicationMenu] = QChar('N');
    s_buttonNames[KDecoration2::DecorationButtonType::OnAllDesktops]   = QChar('S');
    s_buttonNames[KDecoration2::DecorationButtonType::ContextHelp]     = QChar('H');
    s_buttonNames[KDecoration2::DecorationButtonType::Minimize]        = QChar('I');
    s_buttonNames[KDecoration2::DecorationButtonType::Maximize]        = QChar('A');
    s_buttonNames[KDecoration2::DecorationButtonType::Close]           = QChar('X');
    s_buttonNames[KDecoration2::DecorationButtonType::KeepAbove]       = QChar('F');
    s_buttonNames[KDecoration2::DecorationButtonType::KeepBelow]       = QChar('B');
    s_buttonNames[KDecoration2::DecorationButtonType::Shade]           = QChar('L');
}

static QString buttonsToString(const QVector<KDecoration2::DecorationButtonType> &buttons)
{
    auto buttonToString = [](KDecoration2::DecorationButtonType button) -> QChar {
        const auto it = s_buttonNames.constFind(button);
        if (it != s_buttonNames.constEnd()) {
            return it.value();
        }
        return QChar();
    };
    QString ret;
    for (auto button : buttons) {
        ret.append(buttonToString(button));
    }
    return ret;
}

QVector< KDecoration2::DecorationButtonType > SettingsImpl::readDecorationButtons(const KConfigGroup &config,
                                                                                    const char *key,
                                                                                    const QVector< KDecoration2::DecorationButtonType > &defaultValue) const
{
    initButtons();
    auto buttonsFromString = [](const QString &buttons) -> QVector<KDecoration2::DecorationButtonType> {
        QVector<KDecoration2::DecorationButtonType> ret;
        for (auto it = buttons.begin(); it != buttons.end(); ++it) {
            for (auto it2 = s_buttonNames.constBegin(); it2 != s_buttonNames.constEnd(); ++it2) {
                if (it2.value() == (*it)) {
                    ret << it2.key();
                }
            }
        }
        return ret;
    };
    return buttonsFromString(config.readEntry(key, buttonsToString(defaultValue)));
}

static KDecoration2::BorderSize stringToSize(const QString &name)
{
    static const QMap<QString, KDecoration2::BorderSize> s_sizes = QMap<QString, KDecoration2::BorderSize>({
        {QStringLiteral("None"), KDecoration2::BorderSize::None},
        {QStringLiteral("NoSides"), KDecoration2::BorderSize::NoSides},
        {QStringLiteral("Tiny"), KDecoration2::BorderSize::Tiny},
        {QStringLiteral("Normal"), KDecoration2::BorderSize::Normal},
        {QStringLiteral("Large"), KDecoration2::BorderSize::Large},
        {QStringLiteral("VeryLarge"), KDecoration2::BorderSize::VeryLarge},
        {QStringLiteral("Huge"), KDecoration2::BorderSize::Huge},
        {QStringLiteral("VeryHuge"), KDecoration2::BorderSize::VeryHuge},
        {QStringLiteral("Oversized"), KDecoration2::BorderSize::Oversized}
    });
    auto it = s_sizes.constFind(name);
    if (it == s_sizes.constEnd()) {
        // non sense values are interpreted just like normal
        return KDecoration2::BorderSize::Normal;
    }
    return it.value();
}

void SettingsImpl::onGSettingChangedSlot()
{
    fputs("SettingsImpl::onGSettingChangedSlot,  gsetting 生效吗\n", stderr);
    if (false == m_pSettings->keys().contains("systemFontSize"))
    {
        return;
    }

    QString strFontSize = m_pSettings->get("system-font-size").toString();
    bool bOk;
    int nFontSize = strFontSize.toInt(&bOk);
    if(false == bOk)
    {
        return;
    }

    m_font.setPixelSize(nFontSize);
    emit decorationSettings()->fontChanged(m_font);
    return;
}

void SettingsImpl::onFontChanged(int nFont)
{
    m_font.setPixelSize(nFont * m_nScaleFactor);
    emit decorationSettings()->fontChanged(m_font);
    return;
}

void SettingsImpl::readSettings()
{
    KConfigGroup config = kwinApp()->config()->group(QStringLiteral("org.kde.kdecoration2"));
    const auto &left = readDecorationButtons(config, "ButtonsOnLeft", QVector<KDecoration2::DecorationButtonType >({
        KDecoration2::DecorationButtonType::Menu,
        KDecoration2::DecorationButtonType::OnAllDesktops
    }));
    if (left != m_leftButtons) {
        m_leftButtons = left;
        emit decorationSettings()->decorationButtonsLeftChanged(m_leftButtons);
    }
    const auto &right = readDecorationButtons(config, "ButtonsOnRight", QVector<KDecoration2::DecorationButtonType >({
        KDecoration2::DecorationButtonType::ContextHelp,
        KDecoration2::DecorationButtonType::Minimize,
        KDecoration2::DecorationButtonType::Maximize,
        KDecoration2::DecorationButtonType::Close
    }));
    if (right != m_rightButtons) {
        m_rightButtons = right;
        emit decorationSettings()->decorationButtonsRightChanged(m_rightButtons);
    }
    ApplicationMenu::self()->setViewEnabled(left.contains(KDecoration2::DecorationButtonType::ApplicationMenu) || right.contains(KDecoration2::DecorationButtonType::ApplicationMenu));
    const bool close = config.readEntry("CloseOnDoubleClickOnMenu", false);
    if (close != m_closeDoubleClickMenu) {
        m_closeDoubleClickMenu = close;
        emit decorationSettings()->closeOnDoubleClickOnMenuChanged(m_closeDoubleClickMenu);
    }
    m_autoBorderSize = config.readEntry("BorderSizeAuto", true);

    auto size = stringToSize(config.readEntry("BorderSize", QStringLiteral("Normal")));
    if (m_autoBorderSize) {
        /* Falls back to Normal border size, if the plugin does not provide a valid recommendation. */
        size = stringToSize(DecorationBridge::self()->recommendedBorderSize());
    }
    if (size != m_borderSize) {
        m_borderSize = size;
        emit decorationSettings()->borderSizeChanged(m_borderSize);
    }

    QFont font = QFontDatabase::systemFont(QFontDatabase::TitleFont);
    font.setPixelSize(DEFAULT_FONTSIZE * m_nScaleFactor);
    if (font != m_font) {
        m_font = font;
        emit decorationSettings()->fontChanged(m_font);
    }

    emit decorationSettings()->reconfigured();
}

}
}
