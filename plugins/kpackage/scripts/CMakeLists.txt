add_definitions(-DTRANSLATION_DOMAIN=\"kwin_package_scripts\")

set(scripts_SRCS
    scripts.cpp
)

add_library(ukui_kwin_packagestructure_scripts MODULE ${scripts_SRCS})

target_link_libraries(ukui_kwin_packagestructure_scripts
   KF5::I18n
   KF5::Package
)

kcoreaddons_desktop_to_json(ukui_kwin_packagestructure_scripts ukui-kwin-packagestructure-scripts.desktop)

install(TARGETS ukui_kwin_packagestructure_scripts DESTINATION ${KDE_INSTALL_PLUGINDIR}/kpackage/packagestructure)
