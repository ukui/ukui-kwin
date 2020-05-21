#include "gsettingfont.h"
#include <QVariant>

GSettingFont::GSettingFont(QObject *parent)
{
    fputs("GSettingFont::GSettingFont,  构造\n", stderr);

}

void GSettingFont::setFontSize(int nFont)
{
    nFont = nFont + 4;              //标题栏字体的大小实际比真实字体小4个单位
    emit Sig_fontChanged(nFont);
    return;
}

