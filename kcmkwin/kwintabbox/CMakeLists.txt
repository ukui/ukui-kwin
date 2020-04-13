# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_ukuikwintabbox\")

include_directories(${UKUI_KWIN_SOURCE_DIR}/effects ${UKUI_KWIN_SOURCE_DIR}/tabbox ${UKUI_KWIN_SOURCE_DIR})

########### next target ###############

set(kcm_ukuikwintabbox_PART_SRCS
    ${UKUI_KWIN_SOURCE_DIR}/tabbox/tabboxconfig.cpp
    layoutpreview.cpp
    main.cpp
    thumbnailitem.cpp
)

ki18n_wrap_ui(kcm_ukuikwintabbox_PART_SRCS main.ui)
qt5_add_dbus_interface(kcm_ukuikwintabbox_PART_SRCS ${UKUI_KWIN_SOURCE_DIR}/org.ukui.kwin.Effects.xml ukui_kwin_effects_interface)

add_library(kcm_ukuikwintabbox MODULE ${kcm_ukuikwintabbox_PART_SRCS})

target_link_libraries(kcm_ukuikwintabbox
    Qt5::Quick

    KF5::Completion
    KF5::GlobalAccel
    KF5::I18n
    KF5::KCMUtils
    KF5::NewStuff
    KF5::Package
    KF5::Service

    XCB::XCB

    kwin4_effect_builtins
)

install(TARGETS kcm_ukuikwintabbox DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install(FILES ukuikwintabbox.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES thumbnails/konqueror.png thumbnails/kmail.png thumbnails/systemsettings.png thumbnails/dolphin.png DESTINATION ${DATA_INSTALL_DIR}/ukui-kwin/kcm_ukuikwintabbox)
install(FILES ukui-kwinswitcher.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
