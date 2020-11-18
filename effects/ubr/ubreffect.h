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

#ifndef UBREFFECT_H
#define UBREFFECT_H

#include <kwineffects.h>

class UBREffect : public KWin::Effect
{
    Q_OBJECT
public:
    enum DataRole {
        IgnoreUBR = 1000, // used in edged or maximized window, if not valid, skip this effect
        UnityBorderRadius,
        IsCSD, // indicate if window has a common title bar
        IsUKUIDecoration
    };
    Q_ENUM (DataRole)

    explicit UBREffect(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    ~UBREffect();

    void prePaintScreen(KWin::ScreenPrePaintData &data, int time) override;
    void prePaintWindow(KWin::EffectWindow* w, KWin::WindowPrePaintData& data, int time) override;
    void drawWindow(KWin::EffectWindow* w, int mask, const QRegion& region, KWin::WindowPaintData& data) override;

private:
    KWin::GLShader *m_ubrShader = nullptr;
};

#endif // UBREFFECT_H
