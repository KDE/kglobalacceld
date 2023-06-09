cmake_minimum_required(VERSION 3.16)

project(kglobalacceld)

set(PROJECT_VERSION "5.27.80")
set(PROJECT_VERSION_MAJOR 5)

set(QT_MIN_VERSION "6.4.0")
set(KF_DEP_VERSION "5.240.0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FeatureSummary)
find_package(ECM ${KF_DEP_VERSION}  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

cmake_policy(SET CMP0048 NEW)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMGenerateExportHeader)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMPoQmTools)
include(ECMDeprecationSettings)
include(KDEClangFormat)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")


find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED DBus Gui Widgets)

find_package(KF6Config ${KF_DEP_VERSION} REQUIRED)
find_package(KF6CoreAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF6Crash ${KF_DEP_VERSION} REQUIRED)
find_package(KF6DBusAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF6WindowSystem ${KF_DEP_VERSION} REQUIRED)
find_package(KF6GlobalAccel ${KF_DEP_VERSION} REQUIRED)
find_package(KF6Service ${KF_DEP_VERSION} REQUIRED)
find_package(KF6KIO ${KF_DEP_VERSION} REQUIRED)
find_package(KF6Notifications ${KF_DEP_VERSION} REQUIRED)

find_package(XCB MODULE COMPONENTS XCB KEYSYMS XKB RECORD OPTIONAL_COMPONENTS XTEST)
set_package_properties(XCB PROPERTIES DESCRIPTION "X protocol C-language Binding"
                    TYPE OPTIONAL
                    )

find_package(X11)

set(HAVE_X11 0)

if(X11_FOUND AND XCB_XCB_FOUND)
    set(HAVE_X11 1)
endif()

ecm_setup_version(PROJECT VARIABLE_PREFIX KGLOBALACCELD
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KGlobalAccelDConfigVersion.cmake"
                        SOVERSION 0)

ecm_set_disabled_deprecation_versions(
    QT 6.4.0
    KF 5.102.0
)

add_subdirectory(src)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KGlobalAccelD")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KGlobalAccelDConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KGlobalAccelDConfig.cmake"
    PATH_VARS KDE_INSTALL_DBUSINTERFACEDIR
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KGlobalAccelDConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KGlobalAccelDConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KGlobalAccelDTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KGlobalAccelDTargets.cmake NAMESPACE K:: )


install(FILES ${kglobalaccel_version_header}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KGlobalAccel COMPONENT Devel)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
