set(ubrconfig_SRCS ubrconfig.cpp)
ki18n_wrap_ui(ubrconfig_SRCS ubrconfig.ui)

set(INSTALL_DIR "${PLUGIN_INSTALL_DIR}/ukui-kwin/effects/plugins")
set(CONFIG_INSTALL_DIR "${PLUGIN_INSTALL_DIR}/ukui-kwin/effects/configs")



add_library(
    kwin_effect_ubr MODULE
    ubreffect.cpp
    ubreffect.h
    ubr.desktop
    xatom-helper.cpp
    xatom-helper.h
    ubrtexturehelper.cpp
    ubrtexturehelper.h
    shaderhelper.cpp
    shaderhelper.h
)

target_link_libraries(
    kwin_effect_ubr
    PUBLIC
    Qt5::Core Qt5::Gui
    PRIVATE
    KF5::CoreAddons KF5::WindowSystem KF5::ConfigCore KF5::WindowSystem Qt5::X11Extras
    kwineffects
    kwinglutils
    X11
)

install(TARGETS kwin_effect_ubr DESTINATION ${INSTALL_DIR})

add_library(
    kwin_ubr_config MODULE
    ubrconfig.cpp
    ubrconfig.h
    ubrconfig.ui
    ubr_config.desktop
)

target_link_libraries(
    kwin_ubr_config
    PUBLIC
    Qt5::Core Qt5::Gui
    PRIVATE
    KF5::I18n
    KF5::CoreAddons KF5::WindowSystem KF5::ConfigCore KF5::ConfigGui KF5::ConfigWidgets
)

kcoreaddons_desktop_to_json(kwin_ubr_config ubr_config.desktop SERVICE_TYPE kcmodule.desktop)
kcoreaddons_desktop_to_json(kwin_effect_ubr ubr.desktop SERVICE_TYPES kwineffect.desktop)

install(
    TARGETS
    kwin_ubr_config
    DESTINATION
    ${CONFIG_INSTALL_DIR}
)
