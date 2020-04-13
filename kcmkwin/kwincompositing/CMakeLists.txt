#########################################################################
# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmukuikwincompositing\")

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY)

################# configure checks and create the configured files #################

set(ukuikwincomposing_SRC
    main.cpp
    compositing.cpp
)

qt5_add_dbus_interface(ukuikwincomposing_SRC
    ${UKUI_KWIN_SOURCE_DIR}/org.ukui.kwin.Compositing.xml ukui_kwin_compositing_interface
)

ki18n_wrap_ui(ukuikwincomposing_SRC compositing.ui)

add_library(ukuikwincompositing MODULE ${ukuikwincomposing_SRC})

target_link_libraries(ukuikwincompositing
    Qt5::DBus
    Qt5::Widgets

    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::KCMUtils
)

install(TARGETS ukuikwincompositing DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES ukuikwincompositing.desktop DESTINATION ${SERVICES_INSTALL_DIR})
