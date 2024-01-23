install(
    FILES
        kglobalacceld.upd
    DESTINATION
        ${KDE_INSTALL_KCONFUPDATEDIR}
)

install(
    PROGRAMS
        plasma6.0-move-desktop-components-to-services.sh
    DESTINATION
        ${KDE_INSTALL_KCONFUPDATEDIR}
)
