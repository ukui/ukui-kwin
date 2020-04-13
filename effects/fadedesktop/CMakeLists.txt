install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/${UKUI_KWIN_NAME}/effects/kwin4_effect_fadedesktop)

install(FILES package/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}/${UKUI_KWIN_NAME}
        RENAME kwin4_effect_fadedesktop.desktop)

file(COPY package/ DESTINATION ${CMAKE_BINARY_DIR}/bin/ukui-kwin/effects/kwin4_effect_fadedesktop)
