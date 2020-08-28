# kwin-style-ukui -- UKUI KWin Decoration Project

## Introduction & Background
KDE is a well-known desktop environment in the worldwide. Its window manager - KWin is also an amayzing project, which providing many cool effects, and allowding us to extend it with plugins or scripts.

The ukui-kwin-style is based on the KDecoration2, which is an extensible frameworks for KWin to custom a window decoration style, such as title bar, border and shadow. The projects similar with this one are kwin-style-breeze and kwin-decoration-oxygen, etc... Breeze and Oxygen are also providing the applications style based on Qt style frameworks, infuenting most Qt applications.

This project has 2 main targets:
1. providing the decorations for normal windows, and keep the decorations suiting for ukui styles.
2. providing the decorations for special windows (a client side decoration window), such as peony, ukui-control-center, decorating special windows with shadow and a resize handler.

The difference between normal windows and special windows (csd windows) in X11 is window's motif hints atom, which is a protocol of NETWM. However, there are many arguments about client side decoration, so breeze and oxygen choose to ignore some flags of motif hints (actually not handled, motif hints only handled at kwin self), which related csd.

As a developer, I'd like to stand by ssd (sever side decoration) side, but I have to meet the needs of UI designers. I have tried many ways to archives those goals, and as rebasing the rules as possible. I am still looking for the best solution, and this project may just be a step on the way. Actually I don't think I have found a good solution for resolve the arguments on csd, and I gusse it will continue a loog while.

## HOW it works?
In X Window System, there are many protocol between X client (a window) and X sever. Window Manager take over the works of X sever in current PC (In x11 platform), and handle these protocols.

For now, **ICCCM** and **NETWM** is the common protocols in different X11 desktop environment. One protocol of NETWM -- **motif hints** is a protocol to control the window decoration and supported actions provided by window manager.

Throughing motif hints, window can define a rules telling window manager how to decorate it, and enable/disable window interaction actions.

KWin's decoration frameworks, KDecoration2, provides us the convinience access between window and window manger. It let us just need consider how to render decoration for a window. For a decoration to be painted, it can get the window handle from client() which provided by the frameworks. So, we can check the window motif hints durring rendering it. 

For a csd window, the windows motif hints should be different with the normal one, this is mostly handled by gui libraries, such as Qt and GTK. In X11 platform, Qt's frameless window and GTK's csd window both change the motif hints of the window, but their hints are different, that cause a csd window seems better than frameless one. For example, csd window can resize without any hacking, and also has shadow rounded.

The task is taked over by ourself. Implement a KDecoration2 plugin, and use it to handle the different window (note that some window will not handled in plugin, such as frameless window, and gtk csd window, it handled by kwin itself).

If you want to learn more about kwin, the [**offical document**](https://community.kde.org/KWin) is recommend.

## Build and Test
- Dependencies (in debian):
   - cmake
   - extra-cmake-modules
   - libkdecorations2-dev
   - libqt5x11extras5-dev
   - libkf5coreaddons-dev
   - libkf5windowsystem-dev

- Build and Install
   - clone this project and enter into top directory
   - mkdir build && cd build
   - cmake ..
   - make
   - sudo make install
   - ./test-csd (if use kwin-style-ukui)

- Test
   - open KDE System Settings (systemsettings5)
   - Apperance > Application Style > Window Decoration
   - choose UKUI as decoration and apply
   - run a demo, which windows' motif hints is different.

## ScreenShot
![pictrue1](screenshots/kwin-style-ukui-deco-with-different-motif-hints.png)

![pictrue2](screenshots/csd-window.png)

![pictrue3](screenshots/window-screenshot-of-csd-window.png)

## TODO
- Basic decoration rendering. Not only be a demo.
- Support unity border radius protocol, and handle the rounded-corner window's shadow.
- Register an atom to _NET_SUPPORTED. Let window know if kwin-ukui-style used.
- An implement in wayland?