add_subdirectory(code)

install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/${UKUI_KWIN_NAME}/decorations/kwin4_decoration_qml_plastik)

install(FILES package/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}/${UKUI_KWIN_NAME}
        RENAME kwin4_decoration_qml_plastik.desktop)

file(COPY package/ DESTINATION ${CMAKE_BINARY_DIR}/bin/ukui-kwin/decorations/kwin4_decoration_qml_plastik)
