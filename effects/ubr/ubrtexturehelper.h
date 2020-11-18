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

#ifndef UBRTEXTUREHELPER_H
#define UBRTEXTUREHELPER_H

#include <QObject>
#include <QMap>

#include <kwinglutils.h>

class UBRTextureHelper : public QObject
{
    Q_OBJECT
public:
    static UBRTextureHelper *getInstance();
    KWin::GLTexture *getTexture(int borderRadius);

    void release();

private:
    explicit UBRTextureHelper(QObject *parent = nullptr);

    QMap<int, KWin::GLTexture *> m_textures;
};

#endif // UBRTEXTUREHELPER_H
