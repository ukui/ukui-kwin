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

#include <QApplication>

#include <QMainWindow>
#include <QDebug>

#include "xatom-helper.h"

#include <QGraphicsEffect>

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
class WindowEffect : public QGraphicsEffect
{
public:
    explicit WindowEffect(QWidget *parent = nullptr);

    void draw(QPainter *painter) override;
};


WindowEffect::WindowEffect(QWidget *parent) : QGraphicsEffect(parent)
{
    parent->setAttribute(Qt::WA_TranslucentBackground);
    //parent->setWindowFlag(Qt::FramelessWindowHint);
    parent->setGraphicsEffect(this);
}

void WindowEffect::draw(QPainter *painter)
{
    QPoint offset;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    if (sourceIsPixmap()) {
        // No point in drawing in device coordinates (pixmap will be scaled anyways).
        const QPixmap pixmap = sourcePixmap(Qt::LogicalCoordinates, &offset);
        painter->translate(offset);
        QPainterPath path;
        path.addRoundedRect(pixmap.rect(), 0, 0);
        painter->setClipPath(path);
        painter->fillRect(pixmap.rect(), qApp->palette().window());
        painter->drawPixmap(QPoint(), pixmap);
    } else {
        // Draw pixmap in device coordinates to avoid pixmap scaling;
        const QPixmap pixmap = sourcePixmap(Qt::DeviceCoordinates, &offset);
        painter->setWorldTransform(QTransform());
        painter->translate(offset);
        QPainterPath path;
        path.addRoundedRect(pixmap.rect(), 0, 0);
        painter->setClipPath(path);
        painter->fillRect(pixmap.rect(), qApp->palette().window());
        painter->drawPixmap(QPoint(), pixmap);
    }
    painter->restore();
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QMainWindow w;
    w.setProperty("doNotBlur", true);

    WindowEffect e(&w);

    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(w.winId(), hints);

    UnityCorners corners;
    corners.topLeft = 12;
    corners.topRight = 12;
    corners.bottomLeft = 12;
    corners.bottomRight = 12;
    // XAtomHelper::getInstance()->setWindowBorderRadius(w.winId(), corners);
    // auto result = XAtomHelper::getInstance()->getWindowBorderRadius(w.winId());
    // qDebug()<<result.topLeft<<result.topRight<<result.bottomLeft<<result.bottomRight;
    XAtomHelper::getInstance()->setUKUIDecoraiontHint(w.winId(), true);
    //w.setWindowFlag(Qt::FramelessWindowHint);
    w.show();

    return a.exec();
}
