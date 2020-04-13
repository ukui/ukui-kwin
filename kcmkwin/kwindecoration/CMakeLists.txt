# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_ukuikwindecoration\")

add_subdirectory(declarative-plugin)

set(kcmukuikwindecoration_SRCS
    declarative-plugin/buttonsmodel.cpp
    decorationmodel.cpp
    kcm.cpp
    utils.cpp
)

kconfig_add_kcfg_files(kcmukuikwindecoration_SRCS ukuikwindecorationsettings.kcfgc GENERATE_MOC)

add_library(kcm_ukuikwindecoration MODULE ${kcmukuikwindecoration_SRCS})

target_link_libraries(kcm_ukuikwindecoration
    KDecoration2::KDecoration
    KF5::I18n
    KF5::NewStuff
    KF5::QuickAddons
    Qt5::Quick
)

kcoreaddons_desktop_to_json(kcm_ukuikwindecoration "ukuikwindecoration.desktop" SERVICE_TYPES kcmodule.desktop)

# This desktop file is installed only for retrocompatibility with sycoca
install(FILES ukuikwindecorationsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES ukuikwindecoration.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES ukui-window-decorations.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
install(TARGETS kcm_ukuikwindecoration DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)

kpackage_install_package(package kcm_ukuikwindecoration kcms)
