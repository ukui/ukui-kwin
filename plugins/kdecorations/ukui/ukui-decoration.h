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

#ifndef UKUIDECORATION_H
#define UKUIDECORATION_H

#include <KDecoration2/Decoration>
#include <KPluginFactory>

namespace KDecoration2 {
class DecorationButtonGroup;
}

namespace UKUI {

struct ShadowParams {
    ShadowParams()
        : offset(QPoint(0, 0))
        , radius(0)
        , opacity(0) {}

    ShadowParams(const QPoint &offset, int radius, qreal opacity)
        : offset(offset)
        , radius(radius)
        , opacity(opacity) {}

    QPoint offset;
    int radius;
    qreal opacity;
};

struct CompositeShadowParams {
    CompositeShadowParams() = default;

    CompositeShadowParams(
            const QPoint &offset,
            const ShadowParams &shadow1,
            const ShadowParams &shadow2)
        : offset(offset)
        , shadow1(shadow1)
        , shadow2(shadow2) {}

    bool isNone() const {
        return qMax(shadow1.radius, shadow2.radius) == 0;
    }

    QPoint offset;
    ShadowParams shadow1;
    ShadowParams shadow2;
};

class Decoration : public KDecoration2::Decoration
{
    Q_OBJECT
public:
    explicit Decoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    void init();
    void paint(QPainter *painter, const QRect &repaintRegion) override;

    QColor fontColor() const;
    QColor frameColor() const;
    int themeId(){return m_themeId;}

private:
    int m_borderLeft;
    int m_borderTop;
    int m_borderRight;
    int m_borderBottom;

    int m_buttonWidth;          //按钮宽度
    int m_buttonHeight;         //按钮高度

    int m_leftButtonWidth;      //左侧按钮高度
    int m_leftButtonHeight;     //左侧按钮宽度

    int m_ButtonMarginTop;      //按钮上空白

    int m_buttonSpacing;        //按钮空隙

    QColor m_frameColor;        //框体颜色
    QColor m_fontActiveColor;   //活动字体颜色
    QColor m_fontInactiveColor; //非活动字体颜色

    int m_nleftButtonCout;
    int m_nrightButtonCout;

    int m_themeId;

    KDecoration2::DecorationButtonGroup* m_leftButtons;
    KDecoration2::DecorationButtonGroup* m_rightButtons;

    bool m_decoBorderOnly = false;

public slots:
    void updateButtonsGeometry();
    void calculateBorders();
    void updateTitleBar();
    void updateShadow();
    void themeUpdate(int themeId);

};

}

#endif // UKUIDECORATION_H
