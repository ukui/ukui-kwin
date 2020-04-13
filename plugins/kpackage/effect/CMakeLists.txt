add_definitions(-DTRANSLATION_DOMAIN=\"kwin_package_effect\")

set(effect_SRCS
    effect.cpp
)

add_library(ukui_kwin_packagestructure_effect MODULE ${effect_SRCS})

target_link_libraries(ukui_kwin_packagestructure_effect
   KF5::I18n
   KF5::Package
)

kcoreaddons_desktop_to_json(ukui_kwin_packagestructure_effect ukui-kwin-packagestructure-effect.desktop)

install(TARGETS ukui_kwin_packagestructure_effect DESTINATION ${KDE_INSTALL_PLUGINDIR}/kpackage/packagestructure)
