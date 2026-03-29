add_executable(musicassistant-native
    main.cpp
    maclient.cpp
    playercontroller.cpp
    queuecontroller.cpp
    librarycontroller.cpp
    mediaitemmodel.cpp
    playermodel.cpp
    queueitemmodel.cpp
    sendspinclient.cpp
    audiodecoder.cpp
)

qt_add_qml_module(musicassistant-native
    URI io.github.musicassistant.native
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/NowPlayingPage.qml
        qml/LibraryPage.qml
        qml/QueuePage.qml
        qml/PlayersPage.qml
        qml/SettingsPage.qml
        qml/MediaItemDelegate.qml
        qml/AboutPage.qml
        qml/AlbumGridDelegate.qml
)

target_link_libraries(musicassistant-native PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
    Qt6::Multimedia
    KF6::Kirigami
    KF6::I18n
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::DBusAddons
    KF6::Notifications
    KF6::WindowSystem
    KF6::IconThemes
)

install(TARGETS musicassistant-native ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
