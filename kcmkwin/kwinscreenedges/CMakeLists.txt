# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmukuikwinscreenedges\")

include_directories(${UKUI_KWIN_SOURCE_DIR}/effects)
set(kcm_screenedges_SRCS
    monitor.cpp
    screenpreviewwidget.cpp
)
qt5_add_dbus_interface(kcm_screenedges_SRCS ${UKUI_KWIN_SOURCE_DIR}/org.ukui.kwin.Effects.xml ukui_kwin_effects_interface)

set(kcm_ukuikwinscreenedges_PART_SRCS main.cpp ${kcm_screenedges_SRCS})
ki18n_wrap_ui(kcm_ukuikwinscreenedges_PART_SRCS main.ui)
add_library(kcm_ukuikwinscreenedges MODULE ${kcm_ukuikwinscreenedges_PART_SRCS})
set(kcm_screenedges_LIBS
    Qt5::DBus

    KF5::Completion
    KF5::ConfigCore
    KF5::ConfigWidgets
    KF5::I18n
    KF5::Package
    KF5::Plasma
    KF5::Service

    kwin4_effect_builtins
)
target_link_libraries(kcm_ukuikwinscreenedges ${X11_LIBRARIES} ${kcm_screenedges_LIBS})

set(kcm_ukuikwintouchscreenedges_PART_SRCS touch.cpp ${kcm_screenedges_SRCS})
ki18n_wrap_ui(kcm_ukuikwintouchscreenedges_PART_SRCS touch.ui)
add_library(kcm_ukuikwintouchscreen MODULE ${kcm_ukuikwintouchscreenedges_PART_SRCS})
target_link_libraries(kcm_ukuikwintouchscreen ${X11_LIBRARIES} ${kcm_screenedges_LIBS})

install(TARGETS kcm_ukuikwinscreenedges kcm_ukuikwintouchscreen DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES ukuikwinscreenedges.desktop ukuikwintouchscreen.desktop DESTINATION ${SERVICES_INSTALL_DIR})
