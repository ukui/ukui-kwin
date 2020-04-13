include(ECMQMLModules)
ecm_find_qmlmodule(org.kde.plasma.core 2.0)

# KI18N Translation Domain for this library.
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_ukuikwin_effects\")

########### next target ###############

set(kcm_ukuikwin_effects_PART_SRCS
    kcm.cpp
    effectsfilterproxymodel.cpp
)

add_library(kcm_ukuikwin_effects MODULE ${kcm_ukuikwin_effects_PART_SRCS})

target_link_libraries(kcm_ukuikwin_effects
    Qt5::DBus

    KF5::I18n
    KF5::KCMUtils
    KF5::NewStuff
    KF5::QuickAddons
    KF5::XmlGui

    kcmukuikwincommon
)

kcoreaddons_desktop_to_json(kcm_ukuikwin_effects "kcm_ukuikwin_effects.desktop")

########### install files ###############

install(TARGETS kcm_ukuikwin_effects DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)
install(FILES kcm_ukuikwin_effects.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES ukui-kwineffect.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
kpackage_install_package(package kcm_ukuikwin_effects kcms)
