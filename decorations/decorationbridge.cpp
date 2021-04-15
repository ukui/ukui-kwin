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
#include "decorationbridge.h"
#include "decoratedclient.h"
#include "decorationrenderer.h"
#include "decorations_logging.h"
#include "settings.h"
// KWin core
#include "abstract_client.h"
#include "composite.h"
#include "scene.h"
#include "wayland_server.h"
#include "workspace.h"
#include <config-ukui-kwin.h>

// KDecoration
#include <KDecoration2/Decoration>
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationSettings>

// KWayland
#include <KWayland/Server/server_decoration_interface.h>

// Frameworks
#include <KPluginMetaData>
#include <KPluginLoader>

// Qt
#include <QMetaProperty>
#include <QPainter>
#include <QScreen>
#include <QtDBus>
#include <QGSettings>
#include <QFontDatabase>


namespace KWin
{
namespace Decoration
{

static const QString s_aurorae = QStringLiteral("org.ukui.kwin.aurorae");
static const QString s_pluginDirectory = QStringLiteral("org.kde.kdecoration2");
#if HAVE_BREEZE_DECO
static const QString s_defaultPlugin = QStringLiteral(BREEZE_KDECORATION_PLUGIN_ID);
#else
static const QString s_defaultPlugin = s_aurorae;
#endif

KWIN_SINGLETON_FACTORY(DecorationBridge)

DecorationBridge::DecorationBridge(QObject *parent)
    : KDecoration2::DecorationBridge(parent)
    , m_factory(nullptr)
    , m_blur(false)
    , m_showToolTips(false)
    , m_settings()
    , m_noPlugin(false)
{
    KConfigGroup cg(KSharedConfig::openConfig(), "KDE");

    // try to extract the proper defaults file from a lookandfeel package
    const QString looknfeel = cg.readEntry(QStringLiteral("LookAndFeelPackage"), "org.kde.breeze.desktop");
    m_lnfConfig = KSharedConfig::openConfig(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("plasma/look-and-feel/") + looknfeel + QStringLiteral("/contents/defaults")));

    readDecorationOptions();
}

DecorationBridge::~DecorationBridge()
{
    s_self = nullptr;
}

QString DecorationBridge::readPlugin()
{
    //Try to get a default from look and feel
    KConfigGroup cg(m_lnfConfig, "ukui-kwinrc");
    cg = KConfigGroup(&cg, "org.kde.kdecoration2");
    return kwinApp()->config()->group(s_pluginDirectory).readEntry("library", cg.readEntry("library", s_defaultPlugin));
}

static bool readNoPlugin()
{
    return kwinApp()->config()->group(s_pluginDirectory).readEntry("NoPlugin", false);
}

QString DecorationBridge::readTheme() const
{
    //Try to get a default from look and feel
    KConfigGroup cg(m_lnfConfig, "ukui-kwinrc");
    cg = KConfigGroup(&cg, "org.kde.kdecoration2");
    return kwinApp()->config()->group(s_pluginDirectory).readEntry("theme", cg.readEntry("theme", m_defaultTheme));
}

void DecorationBridge::readDecorationOptions()
{
    m_showToolTips = kwinApp()->config()->group(s_pluginDirectory).readEntry("ShowToolTips", true);
}

void DecorationBridge::init()
{
    using namespace KWayland::Server;
    m_noPlugin = readNoPlugin();
    if (m_noPlugin) {
        if (waylandServer()) {
            waylandServer()->decorationManager()->setDefaultMode(ServerSideDecorationManagerInterface::Mode::None);
        }
        return;
    }
    m_pluginLibraryName = readPlugin();
    qDebug() << "DecorationBridge::init, 打印插件名称:" << m_pluginLibraryName;
    m_settings = QSharedPointer<KDecoration2::DecorationSettings>::create(this);
    initPlugin();
    if (!m_factory) {
        if (m_pluginLibraryName != s_defaultPlugin) {
            // try loading default plugin
            m_pluginLibraryName = s_defaultPlugin;
            initPlugin();
        }
        // default plugin failed to load, try fallback
        if (!m_factory) {
            m_pluginLibraryName = s_aurorae;
            initPlugin();
        }
    }
    if (waylandServer()) {
        waylandServer()->decorationManager()->setDefaultMode(m_factory ? ServerSideDecorationManagerInterface::Mode::Server : ServerSideDecorationManagerInterface::Mode::None);
    }

    //读取dpi值
    m_dpi = 96;
    QScreen *primary = QGuiApplication::primaryScreen();
    if (primary) {
        m_dpi = primary->logicalDotsPerInchX();
        if(m_dpi < 30)
        {
            m_dpi = 30;     //设置一个限度，不允许dpi小于30
        }
    }

    QGSettings* pThemeSettings = new QGSettings("org.ukui.style", "/org/ukui/style/", this);

    QString strTheme;
    if (true == pThemeSettings->keys().contains("styleName")){
        strTheme = pThemeSettings->get("style-name").toString();
        qDebug() << "DecorationBridge::init theme:" << strTheme;
    }

    if("ukui-light" == strTheme)
    {
        m_themeId = 0;
    }
    else if("ukui-dark" == strTheme)
    {
        m_themeId = 1;
    }
    else    //默认ukui-default
    {
        m_themeId = 0;
    }


    const QFont font = QFontDatabase::systemFont(QFontDatabase::TitleFont);
    m_nFont = font.pointSize();
    m_strFontFamily = font.family();

    QDBusConnection::sessionBus().connect(QString(),
                                          QStringLiteral("/KGlobalSettings"),
                                          QStringLiteral("org.kde.KGlobalSettings"),
                                          QStringLiteral("slotThemeChange"),
                                          this, SLOT(slotThemeUpdate(int)));

    QDBusConnection::sessionBus().connect(QString(),
                                          QStringLiteral("/KGlobalSettings"),
                                          QStringLiteral("org.kde.KGlobalSettings"),
                                          QStringLiteral("slotFontChange"),
                                          this, SLOT(fontUpdate(int, QString)));

}

void DecorationBridge::slotThemeUpdate(int themeId)
{
    m_themeId = themeId;
}

void DecorationBridge::fontUpdate(int nfont, QString strFamily)
{
    m_nFont = nfont;
    m_strFontFamily = strFamily;
    QFont font;
    font.setPointSize(nfont);
    font.setFamily(strFamily);

    emit sig_updateFont(font);
}

void DecorationBridge::initPlugin()
{
    const auto offers = KPluginLoader::findPluginsById(s_pluginDirectory, m_pluginLibraryName);
    if (offers.isEmpty()) {
        qCWarning(KWIN_DECORATIONS) << "Could not locate decoration plugin";
        return;
    }
    qCDebug(KWIN_DECORATIONS) << "Trying to load decoration plugin: " << offers.first().fileName();
    KPluginLoader loader(offers.first().fileName());
    qDebug() << "DecorationBridge::initPlugin,  初始化插件:" << offers.first().fileName();
    KPluginFactory *factory = loader.factory();
    if (!factory) {
        qCWarning(KWIN_DECORATIONS) << "Error loading plugin:" << loader.errorString();
    } else {
        m_factory = factory;
        loadMetaData(loader.metaData().value(QStringLiteral("MetaData")).toObject());
    }
}

static void recreateDecorations()
{
    Workspace::self()->forEachAbstractClient([](AbstractClient *c) { c->updateDecoration(true, true); });
}

void DecorationBridge::reconfigure()
{
    readDecorationOptions();

    if (m_noPlugin != readNoPlugin()) {
        m_noPlugin = !m_noPlugin;
        // no plugin setting changed
        if (m_noPlugin) {
            // decorations disabled now
            m_pluginLibraryName = QString();
            delete m_factory;
            m_factory = nullptr;
            m_settings.clear();
        } else {
            // decorations enabled now
            init();
        }
        recreateDecorations();
        return;
    }

    const QString newPlugin = readPlugin();
    if (newPlugin != m_pluginLibraryName) {
        // plugin changed, recreate everything
        auto oldFactory = m_factory;
        const auto oldPluginName = m_pluginLibraryName;
        m_pluginLibraryName = newPlugin;
        initPlugin();
        if (m_factory == oldFactory) {
            // loading new plugin failed
            m_factory = oldFactory;
            m_pluginLibraryName = oldPluginName;
        } else {
            recreateDecorations();
            // TODO: unload and destroy old plugin
        }
    } else {
        // same plugin, but theme might have changed
        const QString oldTheme = m_theme;
        m_theme = readTheme();
        if (m_theme != oldTheme) {
            recreateDecorations();
        }
    }
}

void DecorationBridge::loadMetaData(const QJsonObject &object)
{
    // reset all settings
    m_blur = false;
    m_recommendedBorderSize = QString();
    m_theme = QString();
    m_defaultTheme = QString();

    // load the settings
    const QJsonValue decoSettings = object.value(s_pluginDirectory);
    if (decoSettings.isUndefined()) {
        // no settings
        return;
    }
    const QVariantMap decoSettingsMap = decoSettings.toObject().toVariantMap();
    auto blurIt = decoSettingsMap.find(QStringLiteral("blur"));
    if (blurIt != decoSettingsMap.end()) {
        m_blur = blurIt.value().toBool();
    }
    auto recBorderSizeIt = decoSettingsMap.find(QStringLiteral("recommendedBorderSize"));
    if (recBorderSizeIt != decoSettingsMap.end()) {
        m_recommendedBorderSize = recBorderSizeIt.value().toString();
    }
    findTheme(decoSettingsMap);

    Q_EMIT metaDataLoaded();
}

void DecorationBridge::findTheme(const QVariantMap &map)
{
    auto it = map.find(QStringLiteral("themes"));
    if (it == map.end()) {
        return;
    }
    if (!it.value().toBool()) {
        return;
    }
    it = map.find(QStringLiteral("defaultTheme"));
    m_defaultTheme = it != map.end() ? it.value().toString() : QString();
    m_theme = readTheme();
}

std::unique_ptr<KDecoration2::DecoratedClientPrivate> DecorationBridge::createClient(KDecoration2::DecoratedClient *client, KDecoration2::Decoration *decoration)
{
    return std::unique_ptr<DecoratedClientImpl>(new DecoratedClientImpl(static_cast<AbstractClient*>(decoration->parent()), client, decoration));
}

std::unique_ptr<KDecoration2::DecorationSettingsPrivate> DecorationBridge::settings(KDecoration2::DecorationSettings *parent)
{
    return std::unique_ptr<SettingsImpl>(new SettingsImpl(parent));
}

void DecorationBridge::update(KDecoration2::Decoration *decoration, const QRect &geometry)
{
    // TODO: remove check once all compositors implement it
    if (AbstractClient *c = Workspace::self()->findAbstractClient([decoration] (const AbstractClient *client) { return client->decoration() == decoration; })) {
        if (Renderer *renderer = c->decoratedClient()->renderer()) {
            renderer->schedule(geometry);
        }
    }
}

KDecoration2::Decoration *DecorationBridge::createDecoration(AbstractClient *client)
{
    if (m_noPlugin) {
        return nullptr;
    }
    if (!m_factory) {
        return nullptr;
    }
    QVariantMap args({ {QStringLiteral("bridge"), QVariant::fromValue(this)} });

    if (!m_theme.isEmpty()) {
        args.insert(QStringLiteral("theme"), m_theme);
    }
    args.insert(QStringLiteral("dpi"), m_dpi);  //每创建一个渲染端，就把dpi值带过去，后面每新建一个客户就不需要反复获取获取dpi值

    args.insert(QStringLiteral("themeId"), m_themeId);  //针对UKUI定制的主题id

    args.insert(QStringLiteral("systemFontSize"), m_nFont);     //标题栏字体大小

    args.insert(QStringLiteral("systemFont"), m_strFontFamily); //标题栏字体类型

    auto deco = m_factory->create<KDecoration2::Decoration>(client, QVariantList({args}));
    deco->setSettings(m_settings);
    deco->init();
    return deco;
}

static
QString settingsProperty(const QVariant &variant)
{
    if (QLatin1String(variant.typeName()) == QLatin1String("KDecoration2::BorderSize")) {
        return QString::number(variant.toInt());
    } else if (QLatin1String(variant.typeName()) == QLatin1String("QVector<KDecoration2::DecorationButtonType>")) {
        const auto &b = variant.value<QVector<KDecoration2::DecorationButtonType>>();
        QString buffer;
        for (auto it = b.begin(); it != b.end(); ++it) {
            if (it != b.begin()) {
                buffer.append(QStringLiteral(", "));
            }
            buffer.append(QString::number(int(*it)));
        }
        return buffer;
    }
    return variant.toString();
}

QString DecorationBridge::supportInformation() const
{
    QString b;
    b.append(QStringLiteral("Plugin: %1\n").arg(m_pluginLibraryName));
    b.append(QStringLiteral("Theme: %1\n").arg(m_theme));
    b.append(QStringLiteral("Plugin recommends border size: %1\n").arg(m_recommendedBorderSize.isNull() ? "No" : m_recommendedBorderSize));
    b.append(QStringLiteral("Blur: %1\n").arg(m_blur));
    const QMetaObject *metaOptions = m_settings->metaObject();
    for (int i=0; i<metaOptions->propertyCount(); ++i) {
        const QMetaProperty property = metaOptions->property(i);
        if (QLatin1String(property.name()) == QLatin1String("objectName")) {
            continue;
        }
        b.append(QStringLiteral("%1: %2\n").arg(property.name()).arg(settingsProperty(m_settings->property(property.name()))));
    }
    return b;
}

} // Decoration
} // KWin
