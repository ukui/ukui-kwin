add_definitions(-DTRANSLATION_DOMAIN=\"kwin_package_decoration\")

set(decoration_SRCS
    decoration.cpp
)

add_library(ukui_kwin_packagestructure_decoration MODULE ${decoration_SRCS})

target_link_libraries(ukui_kwin_packagestructure_decoration
   KF5::I18n
   KF5::Package
)

kcoreaddons_desktop_to_json(ukui_kwin_packagestructure_decoration ukui-kwin-packagestructure-decoration.desktop)

install(TARGETS ukui_kwin_packagestructure_decoration DESTINATION ${KDE_INSTALL_PLUGINDIR}/kpackage/packagestructure)

