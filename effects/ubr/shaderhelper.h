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

#ifndef SHADERHELPER_H
#define SHADERHELPER_H

#include <QObject>

#include <kwinglutils.h>

class ShaderHelper : public QObject
{
    Q_OBJECT
public:
    enum Direction {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    static KWin::GLShader *getShader();
    static void releaseShaders();

private:
    explicit ShaderHelper(QObject *parent = nullptr);
};

#endif // SHADERHELPER_H
