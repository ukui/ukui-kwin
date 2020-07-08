# ukui-kwin

![build](https://github.com/ukui/ukui-kwin/workflows/build/badge.svg?branch=master)

The next window manager is name ukui-kwin for UKUI3.0

## Description
Ukui-kwin is forked from kwin.

The first setp we only rename these binaries and data files and change some codes necessarily to make sure it won't conflict with kwin.

The further step we want to make big diffrences on ukui-kwin and feed back to kwin

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


