include(ECMQMLModules)
ecm_find_qmlmodule(org.kde.plasma.core 2.0)

# KI18N Translation Domain for this library.
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_ukuikwin_virtualdesktops\")

########### next target ###############

set(kcm_ukuikwin_virtualdesktops_PART_SRCS
    ../../virtualdesktopsdbustypes.cpp
    animationsmodel.cpp
    desktopsmodel.cpp
    virtualdesktops.cpp
)

kconfig_add_kcfg_files(kcm_ukuikwin_virtualdesktops_PART_SRCS ukuivirtualdesktopssettings.kcfgc GENERATE_MOC)

add_library(kcm_ukuikwin_virtualdesktops MODULE ${kcm_ukuikwin_virtualdesktops_PART_SRCS})

target_link_libraries(kcm_ukuikwin_virtualdesktops
    Qt5::DBus

    KF5::I18n
    KF5::KCMUtils
    KF5::QuickAddons
    KF5::XmlGui

    kcmukuikwincommon
)

kcoreaddons_desktop_to_json(kcm_ukuikwin_virtualdesktops "kcm_ukuikwin_virtualdesktops.desktop")

########### install files ###############

install(FILES ukuivirtualdesktopssettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(TARGETS kcm_ukuikwin_virtualdesktops DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)
install(FILES kcm_ukuikwin_virtualdesktops.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR})
kpackage_install_package(package kcm_ukuikwin_virtualdesktops kcms)
