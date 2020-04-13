add_definitions(-DTRANSLATION_DOMAIN=\"kwin_package_windowswitcher\")

set(windowswitcher_SRCS
    windowswitcher.cpp
)

add_library(ukui_kwin_packagestructure_windowswitcher MODULE ${windowswitcher_SRCS})

target_link_libraries(ukui_kwin_packagestructure_windowswitcher
   KF5::I18n
   KF5::Package
)

kcoreaddons_desktop_to_json(ukui_kwin_packagestructure_windowswitcher ukui-kwin-packagestructure-windowswitcher.desktop)

install(TARGETS ukui_kwin_packagestructure_windowswitcher DESTINATION ${KDE_INSTALL_PLUGINDIR}/kpackage/packagestructure)
