set(kglobalaccelprivate_SRCS
    kglobalacceld.cpp
    kglobalshortcuttrigger.cpp
    kserviceactioncomponent.cpp
    component.cpp
    globalshortcut.cpp
    globalshortcutsregistry.cpp
    globalshortcutcontext.cpp
    sequencehelpers_p.cpp
)

add_library(KGlobalAccelD ${kglobalaccelprivate_SRCS})
add_library(K::KGlobalAccelD ALIAS KGlobalAccelD)

ecm_generate_export_header(KGlobalAccelD
    EXPORT_FILE_NAME kglobalacceld_export.h
    BASE_NAME KGlobalAccelD
    VERSION ${PROJECT_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME KGlobalAccel
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 4.3 5.90
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

ecm_qt_declare_logging_category(KGlobalAccelD
    HEADER logging.h
    IDENTIFIER KGLOBALACCELD
    CATEGORY_NAME kf.globalaccel.kglobalacceld
    DESCRIPTION "KGlobalAccelD"
    EXPORT KGLOBALACCELD
)

target_include_directories(KGlobalAccelD INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KGlobalAccelD>")

target_link_libraries(KGlobalAccelD
    PUBLIC
        KF6::GlobalAccel
    PRIVATE
        Qt6::DBus
        KF6::WindowSystem # KKeyServer
        KF6::CoreAddons   # KAboutData
        KF6::ConfigCore
        KF6::Service
        KF6::KIOGui
        KF6::JobWidgets
)

set_target_properties(KGlobalAccelD PROPERTIES
    VERSION ${KGLOBALACCELD_VERSION}
    SOVERSION ${KGLOBALACCELD_SOVERSION}
    EXPORT_NAME "KGlobalAccelD"
)

target_compile_definitions(KGlobalAccelD PRIVATE -DQDBUS=${qdbus_EXECUTABLE})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kglobalacceld_export.h
    shortcutkeystate.h
    kglobalacceld.h
    kglobalshortcuttrigger.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KGlobalAccelD/ COMPONENT Devel
)

install(TARGETS KGlobalAccelD EXPORT KGlobalAccelDTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
