########### next target ###############

set(ukui_kwin_killer_helper_SRCS killer.cpp)

add_executable(ukui_kwin_killer_helper ${ukui_kwin_killer_helper_SRCS})

target_link_libraries(ukui_kwin_killer_helper
    KF5::AuthCore
    KF5::I18n
    KF5::WidgetsAddons
    Qt5::Widgets
    Qt5::X11Extras
)

install(TARGETS ukui_kwin_killer_helper DESTINATION ${LIBEXEC_INSTALL_DIR})
