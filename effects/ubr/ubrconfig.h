#ifndef UBRCONFIG_H
#define UBRCONFIG_H

#include <QWidget>
#include <KCModule>
#include "ui_ubrconfig.h"

namespace Ui {
class UBRConfig;
}

class UBRConfig : public KCModule
{
    Q_OBJECT

public:
    explicit UBRConfig(QWidget *parent = nullptr, const QVariantList& args = QVariantList());
    ~UBRConfig();

    void save() override;

private:
    Ui::UBRConfig *ui;
};

#endif // UBRCONFIG_H
