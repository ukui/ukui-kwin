########### decoration ###############
include_directories(
    ./lib
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(ukui_kwin5_aurorae_PART_SRCS
    aurorae.cpp
    decorationoptions.cpp
    lib/auroraetheme.cpp
    lib/themeconfig.cpp
)

add_library(ukui_kwin5_aurorae MODULE ${ukui_kwin5_aurorae_PART_SRCS})
set_target_properties(ukui_kwin5_aurorae PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/org.kde.kdecoration2/")

target_link_libraries(ukui_kwin5_aurorae
    KDecoration2::KDecoration
    kwineffects
    KF5::ConfigWidgets
    KF5::I18n
    KF5::Package
    KF5::WindowSystem
    Qt5::Quick
    Qt5::UiTools
)

install(TARGETS ukui_kwin5_aurorae DESTINATION ${PLUGIN_INSTALL_DIR}/org.kde.kdecoration2)

set(ukui_decoration_plugin_SRCS
    colorhelper.cpp
    decorationoptions.cpp
    decorationplugin.cpp
)

add_library(ukui_decorationplugin SHARED ${ukui_decoration_plugin_SRCS})
set_target_properties(ukui_decorationplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/org/ukui/kwin/decoration/")
target_link_libraries(ukui_decorationplugin
    KDecoration2::KDecoration
    KF5::ConfigWidgets
    Qt5::Quick
)
install(TARGETS ukui_decorationplugin DESTINATION ${QML_INSTALL_DIR}/org/ukui/kwin/decoration)

########### install files ###############

install(FILES ukui-aurorae.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
install(
    FILES
        qml/AppMenuButton.qml
        qml/AuroraeButton.qml
        qml/AuroraeButtonGroup.qml
        qml/AuroraeMaximizeButton.qml
        qml/Decoration.qml
        qml/DecorationButton.qml
        qml/MenuButton.qml
        qml/aurorae.qml
    DESTINATION
        ${DATA_INSTALL_DIR}/ukui-kwin/aurorae)
set(QMLFILES
    qml/AppMenuButton.qml
    qml/ButtonGroup.qml
    qml/Decoration.qml
    qml/DecorationButton.qml
    qml/MenuButton.qml
    qml/qmldir
)
install(FILES ${QMLFILES} DESTINATION ${QML_INSTALL_DIR}/org/ukui/kwin/decoration)
file(COPY ${QMLFILES} DESTINATION ${CMAKE_BINARY_DIR}/bin/org/ukui/kwin/decoration/)
install(FILES ukuikwindecoration.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
