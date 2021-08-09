/********************************************************************
 UKUI-KWin - the UKUI3.0 window manager
 This file is part of the UKUI project
 The ukui-kwin is forked from kwin

Copyright (C) 2014-2020 kylinos.cn                                        

 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2016 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "x11_platform.h"
#include "x11cursor.h"
#include "edge.h"
#include "sync_filter.h"
#include "windowselector.h"
#include <config-ukui-kwin.h>
#include <kwinconfig.h>
#if HAVE_EPOXY_GLX
#include "glxbackend.h"
#endif
#if HAVE_X11_XINPUT
#include "xinputintegration.h"
#endif
#include "abstract_client.h"
#include "effects_x11.h"
#include "eglonxbackend.h"
#include "keyboard_input.h"
#include "logging.h"
#include "screens_xrandr.h"
#include "screenedges_filter.h"
#include "options.h"
#include "overlaywindow_x11.h"
#include "non_composited_outline.h"
#include "workspace.h"
#include "x11_decoration_renderer.h"
#include "x11_output.h"
#include "xcbutils.h"

#include <kwinxrenderutils.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KCrash>

#include <QThread>
#include <QOpenGLContext>
#include <QX11Info>
#include <QFile>
#include <QDir>
#include <QGSettings>


#define CPU_INFO	"/proc/cpuinfo"
#define LOW_PERFORMANCE_CPU_LIST	"/etc/xdg/LowPerformanceCPU.list"
#define LOW_VGA_PCI_LIST	"/etc/xdg/LowVgaPci.list"
#define PCIE_DEVICE_PATH	"/sys/bus/pci/devices/"
#define UKUI_TRANSPARENCY_SETTING "org.ukui.control-center.personalise"
#define PERSONALSIE_TRAN_KEY   "transparency"
#define PERSONALSIE_EFFECT_KEY   "effect"


namespace KWin
{

X11StandalonePlatform::X11StandalonePlatform(QObject *parent)
    : Platform(parent)
    , m_x11Display(QX11Info::display())
{
#if HAVE_X11_XINPUT
    if (!qEnvironmentVariableIsSet("KWIN_NO_XI2")) {
        m_xinputIntegration = new XInputIntegration(m_x11Display, this);
        m_xinputIntegration->init();
        if (!m_xinputIntegration->hasXinput()) {
            delete m_xinputIntegration;
            m_xinputIntegration = nullptr;
        } else {
            connect(kwinApp(), &Application::workspaceCreated, m_xinputIntegration, &XInputIntegration::startListening);
        }
    }
#endif
    connect(kwinApp(), &Application::workspaceCreated, this,
        [this] {
            if (Xcb::Extensions::self()->isSyncAvailable()) {
                m_syncFilter = std::make_unique<SyncFilter>();
            }
        }
    );

    setSupportsGammaControl(true);
}

X11StandalonePlatform::~X11StandalonePlatform()
{
    if (m_openGLFreezeProtectionThread) {
        m_openGLFreezeProtectionThread->quit();
        m_openGLFreezeProtectionThread->wait();
        delete m_openGLFreezeProtectionThread;
    }
    if (isReady()) {
        XRenderUtils::cleanup();
    }
}

void X11StandalonePlatform::init()
{
    if (!QX11Info::isPlatformX11()) {
        emit initFailed();
        return;
    }
    XRenderUtils::init(kwinApp()->x11Connection(), kwinApp()->x11RootWindow());
    setReady(true);
    emit screensQueried();
}

Screens *X11StandalonePlatform::createScreens(QObject *parent)
{
    return new XRandRScreens(this, parent);
}

OpenGLBackend *X11StandalonePlatform::createOpenGLBackend()
{
    switch (options->glPlatformInterface()) {
#if HAVE_EPOXY_GLX
    case GlxPlatformInterface:
        if (hasGlx()) {
            return new GlxBackend(m_x11Display);
        } else {
            qCWarning(KWIN_X11STANDALONE) << "Glx not available, trying EGL instead.";
            // no break, needs fall-through
            Q_FALLTHROUGH();
        }
#endif
    case EglPlatformInterface:
        return new EglOnXBackend(m_x11Display);
    default:
        // no backend available
        return nullptr;
    }
}

Edge *X11StandalonePlatform::createScreenEdge(ScreenEdges *edges)
{
    if (m_screenEdgesFilter.isNull()) {
        m_screenEdgesFilter.reset(new ScreenEdgesFilter);
    }
    return new WindowBasedEdge(edges);
}

void X11StandalonePlatform::createPlatformCursor(QObject *parent)
{
    auto c = new X11Cursor(parent, m_xinputIntegration != nullptr);
#if HAVE_X11_XINPUT
    if (m_xinputIntegration) {
        m_xinputIntegration->setCursor(c);
        // we know we have xkb already
        auto xkb = input()->keyboard()->xkb();
        xkb->setConfig(kwinApp()->kxkbConfig());
        xkb->reconfigure();
    }
#endif
}

bool X11StandalonePlatform::requiresCompositing() const
{
    return false;
}

bool X11StandalonePlatform::openGLCompositingIsBroken() const
{
    const QString unsafeKey(QLatin1String("OpenGLIsUnsafe") + (kwinApp()->isX11MultiHead() ? QString::number(kwinApp()->x11ScreenNumber()) : QString()));
    return KConfigGroup(kwinApp()->config(), "Compositing").readEntry(unsafeKey, false);
}

QString X11StandalonePlatform::compositingNotPossibleReason() const
{
    // first off, check whether we figured that we'll crash on detection because of a buggy driver
    KConfigGroup gl_workaround_group(kwinApp()->config(), "Compositing");
    const QString unsafeKey(QLatin1String("OpenGLIsUnsafe") + (kwinApp()->isX11MultiHead() ? QString::number(kwinApp()->x11ScreenNumber()) : QString()));
    if (gl_workaround_group.readEntry("Backend", "OpenGL") == QLatin1String("OpenGL") &&
        gl_workaround_group.readEntry(unsafeKey, false))
        return i18n("<b>OpenGL compositing (the default) has crashed KWin in the past.</b><br>"
                    "This was most likely due to a driver bug."
                    "<p>If you think that you have meanwhile upgraded to a stable driver,<br>"
                    "you can reset this protection but <b>be aware that this might result in an immediate crash!</b></p>"
                    "<p>Alternatively, you might want to use the XRender backend instead.</p>");

    if (!Xcb::Extensions::self()->isCompositeAvailable() || !Xcb::Extensions::self()->isDamageAvailable()) {
        return i18n("Required X extensions (XComposite and XDamage) are not available.");
    }
#if !defined( KWIN_HAVE_XRENDER_COMPOSITING )
    if (!hasGlx())
        return i18n("GLX/OpenGL are not available and only OpenGL support is compiled.");
#else
    if (!(hasGlx()
            || (Xcb::Extensions::self()->isRenderAvailable() && Xcb::Extensions::self()->isFixesAvailable()))) {
        return i18n("GLX/OpenGL and XRender/XFixes are not available.");
    }
#endif
    return QString();
}

bool X11StandalonePlatform::adaptCPUPerformance() const
{
    //从系统文件中读取CPU信息
    QFile file;
    QString strCPUInfoList;
    file.setFileName(CPU_INFO);
    bool bRet = file.open(QIODevice::ReadOnly | QIODevice::Text);
    if(false == bRet)
    {
        file.close();
        return false;
    }
    strCPUInfoList = QString(file.readAll());
    file.close();

    //解析CPU信息，提取model name
    QStringList lines = strCPUInfoList.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    QString strLocalCPUInfo;
    foreach (QString line, lines) {
        if (line.startsWith("model name")) {
            strLocalCPUInfo = line.split(":").at(1).trimmed();
            break;
        }
    }

    if("" == strLocalCPUInfo)
    {
        return false;
    }

    QFile fileConfig;
    fileConfig.setFileName(LOW_PERFORMANCE_CPU_LIST);
    bRet = fileConfig.open(QIODevice::ReadOnly | QIODevice::Text);
    if(false == bRet)
    {
        fileConfig.close();
        return false;
    }
    QString strCPUConfig = QString(fileConfig.readAll());
    fileConfig.close();

    if("" == strCPUConfig)
    {
        return false;
    }

    QStringList lines2 = strCPUConfig.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    bool bXRender = false;
    bool bBlurDisable = false;
    foreach (QString line, lines2) {
        if("[XRender]" == line)
        {
            bXRender = true;
            bBlurDisable = false;
        }
        if("[blurDisable]" == line)
        {
            bXRender = false;
            bBlurDisable = true;
        }
        if (true == bXRender && strLocalCPUInfo.contains(line, Qt::CaseSensitive)) {
            printf("X11StandalonePlatform::adaptCPUPerformance=======匹配\n");
            //低性能CPU型号判断，如果是低性能CPU则将渲染后端设为XRender
            KConfigGroup kwinConfig(KSharedConfig::openConfig("ukui-kwinrc"), "Compositing");
            kwinConfig.writeEntry("Backend", "XRender");
            kwinConfig.sync();
            return true;
        }

        if (true == bBlurDisable && strLocalCPUInfo.contains(line, Qt::CaseSensitive)) {
            //显卡差，开多了文件管理器就会卡，需去除毛玻璃
            KConfigGroup kwinConfig(KSharedConfig::openConfig("ukui-kwinrc"), "Plugins");
            kwinConfig.writeEntry("blurEnabled", "false");
            kwinConfig.sync();
        }
    }

    return false;
}

void X11StandalonePlatform::checkJingjiaVga() const
{
    char result[1024] = {0};
    char buf_ps[1024] = {0};
    char cmd[128] = {0};
    FILE *pFile;
    strcpy(cmd, "lspci |grep -i VGA | grep -i Jingjia");
    pFile = popen(cmd, "r");
    if(nullptr != pFile)
    {
        while(fgets(buf_ps, 1024, pFile) != nullptr)
        {
           strncat((char*)result, buf_ps, 1024);
           if(strlen(result) > 0)
           {
               KConfigGroup kConfig(KSharedConfig::openConfig("ukui-kwinrc"), "Plugins");
               kConfig.writeEntry("kwin4_effect_dialogparentEnabled", "false");
               kConfig.sync();
               break;
           }
        }
        pclose(pFile);
        pFile = nullptr;
    }

    FILE *file;
    if (file = fopen("/proc/gpuinfo_0", "r"))
    {
        fclose(file);
        KConfigGroup kConfig(KSharedConfig::openConfig("ukui-kwinrc"), "Plugins");
        kConfig.writeEntry("kwin4_effect_dialogparentEnabled", "false");
        kConfig.sync();
    }

    return;
}

void X11StandalonePlatform::checkQXLVga() const
{
    char result[1024] = {0};
    char buf_ps[1024] = {0};
    char cmd[128] = {0};
    FILE *pFile;
    strcpy(cmd, "lspci |grep -i VGA | grep -i QXL");
    pFile = popen(cmd, "r");
    if(nullptr != pFile)
    {
        while(fgets(buf_ps, 1024, pFile) != nullptr)
        {
           strncat((char*)result, buf_ps, 1024);
           if(strlen(result) > 0)
           {
               KConfigGroup kwinConfig(KSharedConfig::openConfig("ukui-kwinrc"), "Compositing");
               kwinConfig.writeEntry("Backend", "XRender");
               kwinConfig.sync();
               break;
           }
        }
        pclose(pFile);
        pFile = nullptr;
    }

    return;
}

bool X11StandalonePlatform::adaptVga() const
{
    QFile file;
    QString strPreDefinePcieInfo;
    file.setFileName(LOW_VGA_PCI_LIST);
    bool bRet = file.open(QIODevice::ReadOnly | QIODevice::Text);
    if(false == bRet)
    {
        file.close();
        return false;
    }
    strPreDefinePcieInfo = QString(file.readAll());
    file.close();

    //获取预定义pcie信息
    QStringList lines = strPreDefinePcieInfo.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    QMap<int, int> qMapPreDefinePcieInfo;
    bool ok;
    foreach (QString line, lines) {
        QStringList stringList = line.split(":");
        qMapPreDefinePcieInfo.insert(stringList[0].toInt(&ok, 16), stringList[1].toInt(&ok, 16));
        qInfo() <<"X11StandalonePlatform::adaptVga===vid:" << stringList[0] << ", pid:" << stringList[1];
    }


    //根据本机pcie信息遍历匹配预定义信息
    QDir dir(PCIE_DEVICE_PATH);
    if (!dir.exists())
    {
        return false;
    }

    // 遍历PCIE_DEVICE_PATH目录，获取当前PCIE设备列表
    dir.setFilter(QDir::Dirs);
    QStringList busList = dir.entryList();
    busList.removeOne(".");
    busList.removeOne("..");

    foreach(QString bus, busList) {
        QString path;
        QFile file;
        QByteArray charArray;
        bool ok;
        int vid;
        int pid;

        // 读取设备vid
        path = dir.absoluteFilePath(bus + "/" + "vendor");
        file.setFileName(path);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        charArray = file.readAll();
        file.close();
        vid = QString(charArray).toInt(&ok, 16);

        // 读取设备pid
        path = dir.absoluteFilePath(bus + "/" + "device");
        file.setFileName(path);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        charArray = file.readAll();
        file.close();
        pid = QString(charArray).toInt(&ok, 16);

        QMap<int, int>::const_iterator iter1 = qMapPreDefinePcieInfo.find(vid);
        if(iter1 != qMapPreDefinePcieInfo.end() && pid == iter1.value())        //找到,并且vid和pid完全匹配
        {
            qInfo() <<"X11StandalonePlatform::adaptVga=======匹配";
            //显卡vid和pid完全匹配,属于低性能显卡
            KConfigGroup kwinConfig(KSharedConfig::openConfig("ukui-kwinrc"), "Compositing");
            kwinConfig.writeEntry("Backend", "XRender");
            kwinConfig.sync();
            return true;
        }
    }

    return false;
}

bool X11StandalonePlatform::compositingPossible() const
{
    //检查虚拟网卡是不是QXL类型，如果是，则走XRender后端
    //checkQXLVga();
    //检查景嘉微显卡，如果是则将kwin4_effect_dialogparentEnabled特效关闭
	checkJingjiaVga();

    //适配CPU性能或显卡性能,当使用XRender作为渲染后端时,则没有毛玻璃效果,设置一个0.95的透明度
    if(true == adaptCPUPerformance() || true == adaptVga())
    {
        //对于机器性能差的,尤其是某些国产机器,需关闭动画效果,因为显卡等带不起该效果
        KConfigGroup kConfig(KSharedConfig::openConfig("ukui-kwinrc"), "Plugins");
        kConfig.writeEntry("kwin4_effect_maximizeEnabled", "false");
        kConfig.sync();

        if (QGSettings::isSchemaInstalled(UKUI_TRANSPARENCY_SETTING)){
            QGSettings* pTransparency = new QGSettings(UKUI_TRANSPARENCY_SETTING);
            if (pTransparency->keys().contains(PERSONALSIE_TRAN_KEY))
            {
                pTransparency->set(PERSONALSIE_TRAN_KEY, 0.95);
            }
            if (pTransparency->keys().contains(PERSONALSIE_EFFECT_KEY))
            {
                pTransparency->set(PERSONALSIE_EFFECT_KEY, false);
            }
            delete pTransparency;
        }
    }

    // first off, check whether we figured that we'll crash on detection because of a buggy driver
    KConfigGroup gl_workaround_group(kwinApp()->config(), "Compositing");
    const QString unsafeKey(QLatin1String("OpenGLIsUnsafe") + (kwinApp()->isX11MultiHead() ? QString::number(kwinApp()->x11ScreenNumber()) : QString()));
    if (gl_workaround_group.readEntry("Backend", "OpenGL") == QLatin1String("OpenGL") && gl_workaround_group.readEntry(unsafeKey, false))
    {
        //对于从配置文件中读取的参数OpenGLIsUnsafe为true直接忽略
        qInfo() << "X11StandalonePlatform::compositingPossible,  从配置文件中读取的参数OpenGLIsUnsafe为true直接忽略";
        //当读取Backend为OpenGL，并且OpenGLIsUnsafe为true时，由于无法开启毛玻璃，固也设置所有应用的透明度为0.95
        if (QGSettings::isSchemaInstalled(UKUI_TRANSPARENCY_SETTING)){
            QGSettings* pTransparency = new QGSettings(UKUI_TRANSPARENCY_SETTING);
            if (pTransparency->keys().contains(PERSONALSIE_TRAN_KEY))
            {
                pTransparency->set(PERSONALSIE_TRAN_KEY, 0.95);
            }
            if (pTransparency->keys().contains(PERSONALSIE_EFFECT_KEY))
            {
                pTransparency->set(PERSONALSIE_EFFECT_KEY, false);
            }
            delete pTransparency;
        }

        //return false;
    }


    if (!Xcb::Extensions::self()->isCompositeAvailable()) {
        qCDebug(KWIN_CORE) << "No composite extension available";
        return false;
    }
    if (!Xcb::Extensions::self()->isDamageAvailable()) {
        qCDebug(KWIN_CORE) << "No damage extension available";
        return false;
    }
    if (hasGlx())
        return true;
#ifdef KWIN_HAVE_XRENDER_COMPOSITING
    if (Xcb::Extensions::self()->isRenderAvailable() && Xcb::Extensions::self()->isFixesAvailable())
        return true;
#endif
    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGLES) {
        return true;
    } else if (qstrcmp(qgetenv("KWIN_COMPOSE"), "O2ES") == 0) {
        return true;
    }
    qCDebug(KWIN_CORE) << "No OpenGL or XRender/XFixes support";
    return false;
}

bool X11StandalonePlatform::hasGlx()
{
    return Xcb::Extensions::self()->hasGlx();
}

void X11StandalonePlatform::createOpenGLSafePoint(OpenGLSafePoint safePoint)
{
    const QString unsafeKey(QLatin1String("OpenGLIsUnsafe") + (kwinApp()->isX11MultiHead() ? QString::number(kwinApp()->x11ScreenNumber()) : QString()));
    auto group = KConfigGroup(kwinApp()->config(), "Compositing");
    switch (safePoint) {
    case OpenGLSafePoint::PreInit:
        group.writeEntry(unsafeKey, true);
        group.sync();
        // Deliberately continue with PreFrame
        Q_FALLTHROUGH();
    case OpenGLSafePoint::PreFrame:
        if (m_openGLFreezeProtectionThread == nullptr) {
            Q_ASSERT(m_openGLFreezeProtection == nullptr);
            m_openGLFreezeProtectionThread = new QThread(this);
            m_openGLFreezeProtectionThread->setObjectName("FreezeDetector");
            m_openGLFreezeProtectionThread->start();
            m_openGLFreezeProtection = new QTimer;
            m_openGLFreezeProtection->setInterval(15000);
            m_openGLFreezeProtection->setSingleShot(true);
            m_openGLFreezeProtection->start();
            const QString configName = kwinApp()->config()->name();
            m_openGLFreezeProtection->moveToThread(m_openGLFreezeProtectionThread);
            connect(m_openGLFreezeProtection, &QTimer::timeout, m_openGLFreezeProtection,
                [configName] {
                    const QString unsafeKey(QLatin1String("OpenGLIsUnsafe") + (kwinApp()->isX11MultiHead() ? QString::number(kwinApp()->x11ScreenNumber()) : QString()));
                    auto group = KConfigGroup(KSharedConfig::openConfig(configName), "Compositing");
                    group.writeEntry(unsafeKey, true);
                    group.sync();
                    KCrash::setDrKonqiEnabled(false);
                    qFatal("Freeze in OpenGL initialization detected");
                }, Qt::DirectConnection);
        } else {
            Q_ASSERT(m_openGLFreezeProtection);
            QMetaObject::invokeMethod(m_openGLFreezeProtection, "start", Qt::QueuedConnection);
        }
        break;
    case OpenGLSafePoint::PostInit:
        group.writeEntry(unsafeKey, false);
        group.sync();
        // Deliberately continue with PostFrame
        Q_FALLTHROUGH();
    case OpenGLSafePoint::PostFrame:
        QMetaObject::invokeMethod(m_openGLFreezeProtection, "stop", Qt::QueuedConnection);
        break;
    case OpenGLSafePoint::PostLastGuardedFrame:
        m_openGLFreezeProtection->deleteLater();
        m_openGLFreezeProtection = nullptr;
        m_openGLFreezeProtectionThread->quit();
        m_openGLFreezeProtectionThread->wait();
        delete m_openGLFreezeProtectionThread;
        m_openGLFreezeProtectionThread = nullptr;
        break;
    }
}

PlatformCursorImage X11StandalonePlatform::cursorImage() const
{
    auto c = kwinApp()->x11Connection();
    QScopedPointer<xcb_xfixes_get_cursor_image_reply_t, QScopedPointerPodDeleter> cursor(
        xcb_xfixes_get_cursor_image_reply(c,
                                          xcb_xfixes_get_cursor_image_unchecked(c),
                                          nullptr));
    if (cursor.isNull()) {
        return PlatformCursorImage();
    }

    QImage qcursorimg((uchar *) xcb_xfixes_get_cursor_image_cursor_image(cursor.data()), cursor->width, cursor->height,
                      QImage::Format_ARGB32_Premultiplied);
    // deep copy of image as the data is going to be freed
    return PlatformCursorImage(qcursorimg.copy(), QPoint(cursor->xhot, cursor->yhot));
}

void X11StandalonePlatform::doHideCursor()
{
    xcb_xfixes_hide_cursor(kwinApp()->x11Connection(), kwinApp()->x11RootWindow());
}

void X11StandalonePlatform::doShowCursor()
{
    xcb_xfixes_show_cursor(kwinApp()->x11Connection(), kwinApp()->x11RootWindow());
}

void X11StandalonePlatform::startInteractiveWindowSelection(std::function<void(KWin::Toplevel*)> callback, const QByteArray &cursorName)
{
    if (m_windowSelector.isNull()) {
        m_windowSelector.reset(new WindowSelector);
    }
    m_windowSelector->start(callback, cursorName);
}

void X11StandalonePlatform::startInteractivePositionSelection(std::function<void (const QPoint &)> callback)
{
    if (m_windowSelector.isNull()) {
        m_windowSelector.reset(new WindowSelector);
    }
    m_windowSelector->start(callback);
}

void X11StandalonePlatform::setupActionForGlobalAccel(QAction *action)
{
    connect(action, &QAction::triggered, kwinApp(), [action] {
        QVariant timestamp = action->property("org.kde.kglobalaccel.activationTimestamp");
        bool ok = false;
        const quint32 t = timestamp.toULongLong(&ok);
        if (ok) {
            kwinApp()->setX11Time(t);
        }
    });
}

OverlayWindow *X11StandalonePlatform::createOverlayWindow()
{
    return new OverlayWindowX11();
}

/*
 Updates xTime(). This used to simply fetch current timestamp from the server,
 but that can cause xTime() to be newer than timestamp of events that are
 still in our events queue, thus e.g. making XSetInputFocus() caused by such
 event to be ignored. Therefore events queue is searched for first
 event with timestamp, and extra PropertyNotify is generated in order to make
 sure such event is found.
*/
void X11StandalonePlatform::updateXTime()
{
    // NOTE: QX11Info::getTimestamp does not yet search the event queue as the old
    // solution did. This means there might be regressions currently. See the
    // documentation above on how it should be done properly.
    kwinApp()->setX11Time(QX11Info::getTimestamp(), Application::TimestampUpdate::Always);
}

OutlineVisual *X11StandalonePlatform::createOutline(Outline *outline)
{
    // first try composited Outline
    auto ret = Platform::createOutline(outline);
    if (!ret) {
        ret = new NonCompositedOutlineVisual(outline);
    }
    return ret;
}

Decoration::Renderer *X11StandalonePlatform::createDecorationRenderer(Decoration::DecoratedClientImpl *client)
{
    auto renderer = Platform::createDecorationRenderer(client);
    if (!renderer) {
        renderer = new Decoration::X11Renderer(client);
    }
    return renderer;
}

void X11StandalonePlatform::invertScreen()
{
    using namespace Xcb::RandR;
    bool succeeded = false;

    if (Xcb::Extensions::self()->isRandrAvailable()) {
        const auto active_client = workspace()->activeClient();
        ScreenResources res((active_client && active_client->window() != XCB_WINDOW_NONE) ? active_client->window() : rootWindow());

        if (!res.isNull()) {
            for (int j = 0; j < res->num_crtcs; ++j) {
                auto crtc = res.crtcs()[j];
                CrtcGamma gamma(crtc);
                if (gamma.isNull()) {
                    continue;
                }
                if (gamma->size) {
                    qCDebug(KWIN_CORE) << "inverting screen using xcb_randr_set_crtc_gamma";
                    const int half = gamma->size / 2 + 1;

                    uint16_t *red = gamma.red();
                    uint16_t *green = gamma.green();
                    uint16_t *blue = gamma.blue();
                    for (int i = 0; i < half; ++i) {
                        auto invert = [&gamma, i](uint16_t *ramp) {
                            qSwap(ramp[i], ramp[gamma->size - 1 - i]);
                        };
                        invert(red);
                        invert(green);
                        invert(blue);
                    }
                    xcb_randr_set_crtc_gamma(connection(), crtc, gamma->size, red, green, blue);
                    succeeded = true;
                }
            }
        }
    }
    if (!succeeded) {
        Platform::invertScreen();
    }
}

void X11StandalonePlatform::createEffectsHandler(Compositor *compositor, Scene *scene)
{
    new EffectsHandlerImplX11(compositor, scene);
}

QVector<CompositingType> X11StandalonePlatform::supportedCompositors() const
{
    QVector<CompositingType> compositors;
#if HAVE_EPOXY_GLX
    compositors << OpenGLCompositing;
#endif
#ifdef KWIN_HAVE_XRENDER_COMPOSITING
    compositors << XRenderCompositing;
#endif
    compositors << NoCompositing;
    return compositors;
}

void X11StandalonePlatform::initOutputs()
{
    doUpdateOutputs<Xcb::RandR::ScreenResources>();
}

void X11StandalonePlatform::updateOutputs()
{
    doUpdateOutputs<Xcb::RandR::CurrentResources>();
}

template <typename T>
void X11StandalonePlatform::doUpdateOutputs()
{
    auto fallback = [this]() {
        auto *o = new X11Output(this);
        o->setGammaRampSize(0);
        o->setRefreshRate(-1.0f);
        o->setName(QStringLiteral("Xinerama"));
        m_outputs << o;
    };

    // TODO: instead of resetting all outputs, check if new output is added/removed
    //       or still available and leave still available outputs in m_outputs
    //       untouched (like in DRM backend)
    qDeleteAll(m_outputs);
    m_outputs.clear();

    if (!Xcb::Extensions::self()->isRandrAvailable()) {
        fallback();
        return;
    }
    T resources(rootWindow());
    if (resources.isNull()) {
        fallback();
        return;
    }
    xcb_randr_crtc_t *crtcs = resources.crtcs();
    xcb_randr_mode_info_t *modes = resources.modes();

    QVector<Xcb::RandR::CrtcInfo> infos(resources->num_crtcs);
    for (int i = 0; i < resources->num_crtcs; ++i) {
        infos[i] = Xcb::RandR::CrtcInfo(crtcs[i], resources->config_timestamp);
    }

    for (int i = 0; i < resources->num_crtcs; ++i) {
        Xcb::RandR::CrtcInfo info(infos.at(i));

        xcb_randr_output_t *outputs = info.outputs();
        QVector<Xcb::RandR::OutputInfo> outputInfos(outputs ? resources->num_outputs : 0);
        if (outputs) {
            for (int i = 0; i < resources->num_outputs; ++i) {
                outputInfos[i] = Xcb::RandR::OutputInfo(outputs[i], resources->config_timestamp);
            }
        }

        float refreshRate = -1.0f;
        for (int j = 0; j < resources->num_modes; ++j) {
            if (info->mode == modes[j].id) {
                if (modes[j].htotal != 0 && modes[j].vtotal != 0) { // BUG 313996
                    // refresh rate calculation - WTF was wikipedia 1998 when I needed it?
                    int dotclock = modes[j].dot_clock,
                          vtotal = modes[j].vtotal;
                    if (modes[j].mode_flags & XCB_RANDR_MODE_FLAG_INTERLACE)
                        dotclock *= 2;
                    if (modes[j].mode_flags & XCB_RANDR_MODE_FLAG_DOUBLE_SCAN)
                        vtotal *= 2;
                    refreshRate = dotclock/float(modes[j].htotal*vtotal);
                }
                break; // found mode
            }
        }

        const QRect geo = info.rect();
        if (geo.isValid()) {
            xcb_randr_crtc_t crtc = crtcs[i];

            // TODO: Perhaps the output has to save the inherited gamma ramp and
            // restore it during tear down. Currently neither standalone x11 nor
            // drm platform do this.
            Xcb::RandR::CrtcGamma gamma(crtc);

            auto *o = new X11Output(this);
            o->setCrtc(crtc);
            o->setGammaRampSize(gamma.isNull() ? 0 : gamma->size);
            o->setGeometry(geo);
            o->setRefreshRate(refreshRate * 1000);

            QString name;
            for (int j = 0; j < info->num_outputs; ++j) {
                Xcb::RandR::OutputInfo outputInfo(outputInfos.at(j));
                if (crtc == outputInfo->crtc) {
                    name = outputInfo.name();
                    break;
                }
            }
            o->setName(name);
            m_outputs << o;
        }
    }

    if (m_outputs.isEmpty()) {
        fallback();
    }
}

Outputs X11StandalonePlatform::outputs() const
{
    return m_outputs;
}

Outputs X11StandalonePlatform::enabledOutputs() const
{
    return m_outputs;
}

}
