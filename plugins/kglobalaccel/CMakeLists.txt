set(kglobalaccel_plugin_SRCS
    kglobalaccel_plugin.cpp
)

add_library(KF5GlobalAccelPrivateKWin MODULE ${kglobalaccel_plugin_SRCS})
set_target_properties(KF5GlobalAccelPrivateKWin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/org.ukui.kglobalaccel5.platforms/")
target_link_libraries(KF5GlobalAccelPrivateKWin
    KF5::GlobalAccelPrivate
    ukui-kwin
)

install(
    TARGETS
        KF5GlobalAccelPrivateKWin
    DESTINATION
        ${PLUGIN_INSTALL_DIR}/org.ukui.kglobalaccel5.platforms/
)
