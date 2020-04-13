########### next target ###############
# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmukwm\")

set(kcm_ukuikwinoptions_PART_SRCS
    ${UKUI_KWIN_SOURCE_DIR}/effects/effect_builtins.cpp
    main.cpp
    mouse.cpp
    windows.cpp
)

ki18n_wrap_ui(kcm_ukuikwinoptions_PART_SRCS
    actions.ui
    advanced.ui
    focus.ui
    mouse.ui
    moving.ui
)

qt5_add_dbus_interface(kcm_ukuikwinoptions_PART_SRCS ${UKUI_KWIN_SOURCE_DIR}/org.ukui.kwin.Effects.xml ukui_kwin_effects_interface)
add_library(kcm_ukuikwinoptions MODULE ${kcm_ukuikwinoptions_PART_SRCS})
target_link_libraries(kcm_ukuikwinoptions Qt5::DBus KF5::Completion KF5::I18n KF5::ConfigWidgets KF5::Service KF5::WindowSystem)
install(TARGETS kcm_ukuikwinoptions DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(
    FILES
        ukuikwinactions.desktop
        ukuikwinadvanced.desktop
        ukuikwinfocus.desktop
        ukuikwinmoving.desktop
        ukuikwinoptions.desktop
    DESTINATION
        ${SERVICES_INSTALL_DIR}
)
