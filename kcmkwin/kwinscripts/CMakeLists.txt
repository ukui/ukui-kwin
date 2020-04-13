# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm-ukuikwin-scripts\")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(kcm_SRCS
    main.cpp
    module.cpp
)

ki18n_wrap_ui(kcm_SRCS module.ui)

add_library(kcm_ukuikwin_scripts MODULE ${kcm_SRCS})

target_link_libraries(kcm_ukuikwin_scripts
    Qt5::DBus

    KF5::I18n
    KF5::KCMUtils
    KF5::KIOCore
    KF5::NewStuff
    KF5::Package
)

install(TARGETS kcm_ukuikwin_scripts DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES ukuikwinscripts.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES ukui-kwinscripts.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
