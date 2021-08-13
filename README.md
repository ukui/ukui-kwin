# ukui-kwin

![build](https://github.com/ukui/ukui-kwin/workflows/Check%20build/badge.svg?branch=master)

The window manager for UKUI desktop environment.

## Description
Ukui-kwin is the default window manager for UKUI desktop environment, and is forked from kwin.

At first, we forked from kwin, because that Kwin suggest plasma will introduce the whole KDE desktop environment.

Secondly, along with the time of using it, in order to adapt to UKUI's desktop more, we modified some codes, such as some of Qt's components can't minimize when HiddenPreviews was set for 6 in the kwinrc; When you use the showdesktop function in the taskbar, the original dialog box will pop up automatically even you activate only one file; Usually, we just make the effect of blur disable not shutting dowm the composite when the config of OpenGLIsUnsafe was checked for true , etc.

Thirdly, we also need to customize some plug-ins according to our desktop requirements, such as titlebar theme styles. We set some peculiar shutcut, such as swiching workspace. Mouse style will make effect according to our's signal.

Last, We need to introduce special proprietary configuration.

## Dependencies
All of ukui-kwin's dependencies are found through CMake. CMake will report what is missing.
On Debian based distributions the easiest way to install all build dependencies is
```bash
sudo apt build-dep ukui-kwin-wayland
```
## Quick building
  
Ukui-kwin uses CMake. This means that ukui-kwin can be build in a normal cmake-style out of source tree.
```
mkdir build
cd build
cmake ../
make
```

