/********************************************************************
 UKUI-KWin - the UKUI3.0 window manager
 This file is part of the UKUI project
 The ukui-kwin is forked from kwin

Copyright (C) 2014-2020 kylinos.cn

 KWin - the KDE window manager
 This file is part of the KDE project.

 Copyright (C) 2009 Martin Gräßlin <mgraesslin@kde.org>

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
#include "magiclamp_config.h"
// KConfigSkeleton
#include "magiclampconfig.h"
#include <config-ukui-kwin.h>

#include <kwineffects_interface.h>

#include <kconfiggroup.h>
#include <KAboutData>
#include <KPluginFactory>

#include <QVBoxLayout>

K_PLUGIN_FACTORY_WITH_JSON(MagicLampEffectConfigFactory,
                           "magiclamp_config.json",
                           registerPlugin<KWin::MagicLampEffectConfig>();)

namespace KWin
{

MagicLampEffectConfigForm::MagicLampEffectConfigForm(QWidget* parent) : QWidget(parent)
{
    setupUi(this);
}

MagicLampEffectConfig::MagicLampEffectConfig(QWidget* parent, const QVariantList& args) :
    KCModule(KAboutData::pluginData(QStringLiteral("magiclamp")), parent, args)
{
    m_ui = new MagicLampEffectConfigForm(this);

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(m_ui);

    MagicLampConfig::instance(UKUI_KWIN_CONFIG);
    addConfig(MagicLampConfig::self(), m_ui);

    load();
}

void MagicLampEffectConfig::save()
{
    KCModule::save();
    OrgUkuiKwinEffectsInterface interface(QStringLiteral("org.ukui.KWin"),
                                         QStringLiteral("/Effects"),
                                         QDBusConnection::sessionBus());
    interface.reconfigureEffect(QStringLiteral("magiclamp"));
}

} // namespace

#include "magiclamp_config.moc"
