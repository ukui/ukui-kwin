#include "ubrconfig.h"
#include <KPluginFactory>

#include <KAboutData>

K_PLUGIN_FACTORY_WITH_JSON(UBRConfigFactory,
                           "ubr_config.json",
                           registerPlugin<UBRConfig>();)

UBRConfig::UBRConfig(QWidget *parent, const QVariantList &args) :
    KCModule(KAboutData::pluginData(QStringLiteral("ubr")), parent, args),
    ui(new Ui::UBRConfig)
{
    ui->setupUi(this);

    load();
}

UBRConfig::~UBRConfig()
{
    delete ui;
}

void UBRConfig::save()
{
    KCModule::save();
}

#include "ubrconfig.moc"
