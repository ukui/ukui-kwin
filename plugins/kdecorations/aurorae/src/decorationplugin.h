/********************************************************************
 UKUI-KWin - the UKUI3.0 window manager
 This file is part of the UKUI project
 The ukui-kwin is forked from kwin
    
Copyright (C) 2014-2020 kylinos.cn

Copyright (C) 2012 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#ifndef DECORATION_PLUGIN_H
#define DECORATION_PLUGIN_H
#include <QQmlExtensionPlugin>

class DecorationPlugin : public QQmlExtensionPlugin
{
    Q_PLUGIN_METADATA(IID "org.ukui.kwin.decoration")
    Q_OBJECT
public:
    void registerTypes(const char *uri) override;
};

#endif
