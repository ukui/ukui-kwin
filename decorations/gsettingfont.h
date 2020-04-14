#ifndef GSETTINGTHREAD_H
#define GSETTINGTHREAD_H

#include <QObject>
#include <QGSettings>

class GSettingFont : public QObject
{
    Q_OBJECT
    //申明该类有D-BUS服务接口
    Q_CLASSINFO("D-Bus Interface", "com.ukui.kwin")

public:
    GSettingFont(QObject *parent = nullptr);

private:
    QGSettings* m_pSettings;

signals:
    void Sig_fontChanged(int);

public Q_SLOTS:
    void    setFontSize(int nFont);
    void    onGSettingChanged();

};

#endif // GSETTINGTHREAD_H
