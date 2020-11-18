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

#include "ubrtexturehelper.h"

#include <QPainterPath>
#include <QPainter>
#include <QPixmap>

#include <QDebug>

static UBRTextureHelper *global_instance = nullptr;

UBRTextureHelper *UBRTextureHelper::getInstance()
{
    if (!global_instance)
        global_instance = new UBRTextureHelper;
    return global_instance;
}

KWin::GLTexture *UBRTextureHelper::getTexture(int borderRadius)
{
    if (m_textures.value(borderRadius)) {
        return m_textures.value(borderRadius);
    }

    QPixmap pix(QSize(borderRadius, borderRadius));
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.moveTo(borderRadius, 0);
    path.arcTo(0, 0, 2*borderRadius, 2*borderRadius, 90, 90);
    path.lineTo(borderRadius, borderRadius);
    path.lineTo(borderRadius, 0);

    painter.fillPath(path, Qt::white);

    auto texture = new KWin::GLTexture(pix);
    texture->setFilter(GL_LINEAR); //smoother
    texture->setWrapMode(GL_CLAMP_TO_BORDER); //no border color filling

    m_textures.insert(borderRadius, texture);

    return texture;
}

void UBRTextureHelper::release()
{
    for (auto texture : m_textures) {
        texture->clear();
        delete texture;
    }

    m_textures.clear();
}

UBRTextureHelper::UBRTextureHelper(QObject *parent) : QObject(parent)
{

}
