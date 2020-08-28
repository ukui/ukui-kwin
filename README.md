# ukui-kwin

![build](https://github.com/ukui/ukui-kwin/workflows/Check%20build/badge.svg?branch=master)

The window manager for UKUI desktop environment.

## Description
Ukui-kwin is the default window manager for UKUI desktop environment, and is forked from kwin.

The first step we only rename these binaries and data files and change some necessary codes to make sure it won't conflict with kwin.

Second, That Kwin suggest plasma will introduce the whole KDE desktop environment, We don't need it.

Third, In order to adapt to UKUI's desktop. According to our desktop environment requirements, some core code needs to be repaired, otherwise it will affect the use of some other of UKUI's components; we also need to customize some plug-ins according to our desktop requirements, such as shortcut keys, theme styles, mouse patterns, etc.

Last, We need to introduce special proprietary configuration according to our requirements.

## Dependencies
All of ukui-kwin's dependencies are found through CMake. CMake will report what is missing.
On Debian based distributions the easiest way to install all build dependencies is

    sudo apt build-dep ukui-kwin-wayland

## Quick building
  
Ukui-kwin uses CMake. This means that ukui-kwin can be build in a normal cmake-style out of source tree.

    mkdir build
    cd build
    cmake ../
    make


