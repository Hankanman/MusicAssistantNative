cmake_minimum_required(VERSION 3.20)

project(musicassistant-native VERSION 2026.04.08 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "6.6.0")
set(KF_MIN_VERSION "6.0.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(FeatureSummary)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX KDEMUSICASSISTANT
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/version.h"
)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    Quick
    QuickControls2
    Widgets
    Network
    WebSockets
    Multimedia
    DBus
    Qml
)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    Kirigami
    I18n
    CoreAddons
    Config
    DBusAddons
    Notifications
    WindowSystem
    IconThemes
)

add_subdirectory(src)

install(FILES io.github.musicassistant.native.desktop
    DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES io.github.musicassistant.native.metainfo.xml
    DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES icons/musicassistant-native.svg
    DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
