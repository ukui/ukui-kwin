#include <QApplication>

#include <QMainWindow>

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
        path.addRoundedRect(pixmap.rect(), 20, 20);
        painter->setClipPath(path);
        painter->fillRect(pixmap.rect(), qApp->palette().window());
        painter->drawPixmap(QPoint(), pixmap);
    } else {
        // Draw pixmap in device coordinates to avoid pixmap scaling;
        const QPixmap pixmap = sourcePixmap(Qt::DeviceCoordinates, &offset);
        painter->setWorldTransform(QTransform());
        painter->translate(offset);
        QPainterPath path;
        path.addRoundedRect(pixmap.rect(), 20, 20);
        painter->setClipPath(path);
        painter->fillRect(pixmap.rect(), qApp->palette().window());
        painter->drawPixmap(QPoint(), pixmap);
    }
    painter->restore();
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QMainWindow w;

    WindowEffect e(&w);

    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(w.winId(), hints);

    w.show();

    return a.exec();
}
