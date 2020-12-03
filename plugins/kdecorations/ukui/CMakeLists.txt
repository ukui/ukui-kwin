cmake_minimum_required(VERSION 3.5)

message("${PLUGIN_INSTALL_DIR}")
message("${QT_INSTALL_PLUGINS}")

add_definitions(-DCMAKE_INSTALL_PREFIX="/usr")

project(kwin-style-ukui LANGUAGES CXX)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(ECMFindModuleHelpers)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Gui LinguistTools DBus X11Extras REQUIRED)

#set(TS_FILES kwin-style-ukui_zh_CN.ts)

add_library(kwin-style-ukui SHARED
  ukui-decoration.cpp
  ukui-decoration.h
  button.cpp
  button.h
  shadow-helper.cpp
  shadow-helper.h
  xatom-helper.cpp
  xatom-helper.h
  kwin-style-ukui.json
  icon.qrc
  #${TS_FILES}
)

add_executable(test-csd test/test-csd.cpp xatom-helper.cpp xatom-helper.h)

find_package(KDecoration2)
find_package(KF5CoreAddons)
find_package(KF5WindowSystem)
find_package(KF5GuiAddons)
find_package(XCB)
target_link_libraries(kwin-style-ukui PUBLIC KDecoration2::KDecoration KF5::ConfigWidgets KF5::CoreAddons KF5::WindowSystem PRIVATE Qt5::Gui Qt5::DBus Qt5::X11Extras KF5::GuiAddons kwineffects XCB::XCB XCB::UTIL -lX11)
target_link_libraries(test-csd PUBLIC KDecoration2::KDecoration KF5::CoreAddons KF5::WindowSystem PRIVATE Qt5::Gui Qt5::DBus Qt5::X11Extras KF5::GuiAddons XCB::XCB XCB::UTIL -lX11)

#target_compile_definitions(kwin-style-ukui PUBLIC KWINSTYLEUKUI_LIBRARY)

#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

message("${PLUGIN_INSTALL_DIR}")
message("${QT_INSTALL_PLUGINS}")

install(TARGETS kwin-style-ukui DESTINATION ${PLUGIN_INSTALL_DIR}/org.kde.kdecoration2)
