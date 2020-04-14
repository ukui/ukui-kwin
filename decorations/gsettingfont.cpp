#include "gsettingfont.h"
#include <QVariant>

GSettingFont::GSettingFont(QObject *parent)
{
    fputs("GSettingThread::GSettingThread,  构造\n", stderr);
    const QByteArray schema("org.ukui.style");
    const QByteArray path("/org/ukui/style/");
    m_pSettings = new QGSettings(schema, path, this);
    connect(m_pSettings, SIGNAL(changed(const QString&)), this, SLOT(onGSettingChanged()));
}

void GSettingFont::onGSettingChanged()
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

    return;
}

void GSettingFont::setFontSize(int nFont)
{
        fputs("GSettingThread::test,  gsetting 生效吗\n", stderr);
        nFont = nFont + 4;              //标题栏字体的大小实际比真实字体小4个单位
        emit Sig_fontChanged(nFont);
        return;
}

