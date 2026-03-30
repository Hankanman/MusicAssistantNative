import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root

    title: i18n("Music Assistant Native")
    minimumWidth: Kirigami.Units.gridUnit * 30
    minimumHeight: Kirigami.Units.gridUnit * 25
    width: Kirigami.Units.gridUnit * 50
    height: Kirigami.Units.gridUnit * 35

    property bool isConnected: MaClient.authenticated
    property bool showPlayerBar: root.isConnected && PlayerController.currentPlayerId !== ""

    globalDrawer: Kirigami.GlobalDrawer {
        title: i18n("Music Assistant")
        titleIcon: "musicassistant-native"
        isMenu: false
        modal: Kirigami.Settings.isMobile
        collapsible: true
        collapsed: false

        actions: [
            Kirigami.Action {
                text: i18n("Now Playing")
                icon.name: "media-playback-start"
                onTriggered: root.switchPage(nowPlayingPage)
            },
            Kirigami.Action {
                text: i18n("Library")
                icon.name: "view-media-playlist"
                onTriggered: root.switchPage(libraryPage)
            },
            Kirigami.Action {
                text: i18n("Queue")
                icon.name: "amarok_playlist"
                onTriggered: root.switchPage(queuePage)
            },
            Kirigami.Action {
                text: i18n("Players")
                icon.name: "speaker"
                onTriggered: root.switchPage(playersPage)
            },
            Kirigami.Action {
                separator: true
            },
            Kirigami.Action {
                text: i18n("Settings")
                icon.name: "settings-configure"
                onTriggered: root.switchPage(settingsPage)
            },
            Kirigami.Action {
                text: i18n("About")
                icon.name: "help-about"
                onTriggered: root.switchPage(aboutPage)
            }
        ]

        header: ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Icon {
                source: "network-connect"
                Layout.alignment: Qt.AlignHCenter
                implicitWidth: Kirigami.Units.iconSizes.small
                implicitHeight: Kirigami.Units.iconSizes.small
                color: root.isConnected ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.disabledTextColor
            }
            QQC2.Label {
                text: root.isConnected ? (MaClient.serverName || i18n("Connected")) : i18n("Not connected")
                Layout.alignment: Qt.AlignHCenter
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                color: root.isConnected ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
            }
        }
    }

    // Pre-created page instances — avoids "not placed in graphics scene" warnings
    NowPlayingPage { id: nowPlayingPage; visible: false }
    LibraryPage { id: libraryPage; visible: false }
    QueuePage { id: queuePage; visible: false }
    PlayersPage { id: playersPage; visible: false }
    SettingsPage { id: settingsPage; visible: false }
    AboutPage { id: aboutPage; visible: false }

    pageStack.initialPage: settingsPage

    // Auto-select this device (Sendspin player) when it appears in the player list
    Connections {
        target: PlayerModel
        function onCountChanged() {
            if (PlayerModel.count > 0 && PlayerController.currentPlayerId === "") {
                // Prefer our Sendspin local player (prefixed with "up" on server)
                var localId = "up" + SendspinClient.playerId
                for (var i = 0; i < PlayerModel.count; i++) {
                    var pid = PlayerModel.playerIdAt(i)
                    if (pid === localId) {
                        PlayerController.currentPlayerId = pid
                        return
                    }
                }
                // Fallback to first available if local player not found yet
                var firstId = PlayerModel.playerIdAt(0)
                if (firstId !== "") {
                    PlayerController.currentPlayerId = firstId
                }
            }
        }
    }

    function switchPage(page) {
        if (pageStack.currentItem !== page) {
            // Use replace to avoid destroying the old page while Kirigami
            // is still incubating its ActionToolBar
            while (pageStack.depth > 1) pageStack.pop()
            pageStack.replace(page)
        }
    }

    // Keyboard shortcuts
    Shortcut { sequence: "Space"; onActivated: PlayerController.playPause() }
    Shortcut { sequence: "N"; onActivated: PlayerController.next() }
    Shortcut { sequence: "P"; onActivated: PlayerController.previous() }
    Shortcut { sequence: "M"; onActivated: PlayerController.toggleMute() }
    Shortcut { sequence: "Ctrl+Up"; onActivated: PlayerController.volumeUp() }
    Shortcut { sequence: "Ctrl+Down"; onActivated: PlayerController.volumeDown() }

    // Give pageStack bottom margin so content isn't hidden behind player bar
    pageStack.anchors.bottomMargin: root.showPlayerBar ? playerBarContainer.height : 0

    // Persistent bottom player bar — anchored to window contentItem (below sidebar)
    Item {
        id: playerBarContainer
        visible: root.showPlayerBar
        z: 1
        implicitHeight: playerBar.implicitHeight

        parent: root.contentItem
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        // Progress bar at top
        QQC2.ProgressBar {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 3
            from: 0
            to: PlayerController.duration > 0 ? PlayerController.duration : 1
            value: PlayerController.elapsed
            visible: PlayerController.duration > 0
            background: Rectangle { color: Kirigami.Theme.backgroundColor }
            z: 2
        }

    QQC2.ToolBar {
        id: playerBar
        anchors.fill: parent
        padding: Kirigami.Units.smallSpacing
        topPadding: Kirigami.Units.smallSpacing
        bottomPadding: Kirigami.Units.smallSpacing

        contentItem: RowLayout {
            id: playerBarRow
            spacing: Kirigami.Units.smallSpacing

            // Album art thumbnail
            Image {
                source: PlayerController.currentTrackImageUrl !== ""
                    ? PlayerController.currentTrackImageUrl
                    : ""
                Layout.preferredWidth: Kirigami.Units.gridUnit * 3
                Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                fillMode: Image.PreserveAspectCrop
                visible: PlayerController.currentTrackImageUrl !== ""

                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    border.color: Kirigami.Theme.disabledTextColor
                    border.width: 1
                    radius: 3
                }
            }

            // Track info
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                QQC2.Label {
                    text: PlayerController.currentTrackTitle || i18n("Nothing playing")
                    font.bold: true
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                QQC2.Label {
                    text: PlayerController.currentTrackArtist
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    color: Kirigami.Theme.disabledTextColor
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    visible: text !== ""
                }
            }

            // Playback controls
            QQC2.ToolButton {
                icon.name: "media-skip-backward"
                onClicked: PlayerController.previous()
                QQC2.ToolTip.text: i18n("Previous")
                QQC2.ToolTip.visible: hovered
            }
            QQC2.ToolButton {
                icon.name: PlayerController.isPlaying ? "media-playback-pause" : "media-playback-start"
                onClicked: PlayerController.playPause()
                QQC2.ToolTip.text: PlayerController.isPlaying ? i18n("Pause") : i18n("Play")
                QQC2.ToolTip.visible: hovered
            }
            QQC2.ToolButton {
                icon.name: "media-skip-forward"
                onClicked: PlayerController.next()
                QQC2.ToolTip.text: i18n("Next")
                QQC2.ToolTip.visible: hovered
            }

            // Volume popup
            QQC2.ToolButton {
                id: volumeButton
                icon.name: PlayerController.volumeMuted ? "audio-volume-muted"
                         : PlayerController.volumeLevel > 66 ? "audio-volume-high"
                         : PlayerController.volumeLevel > 33 ? "audio-volume-medium"
                         : "audio-volume-low"
                onClicked: volumePopup.open()
                QQC2.ToolTip.text: i18n("Volume: %1%", PlayerController.volumeLevel)
                QQC2.ToolTip.visible: hovered && !volumePopup.visible

                QQC2.Popup {
                    id: volumePopup
                    y: -height - Kirigami.Units.smallSpacing
                    x: (parent.width - width) / 2
                    width: Kirigami.Units.gridUnit * 3
                    height: Kirigami.Units.gridUnit * 12
                    padding: Kirigami.Units.smallSpacing

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: Kirigami.Units.smallSpacing

                        QQC2.Label {
                            text: PlayerController.volumeLevel + "%"
                            font.pointSize: Kirigami.Theme.smallFont.pointSize
                            Layout.alignment: Qt.AlignHCenter
                        }

                        QQC2.Slider {
                            Layout.fillHeight: true
                            Layout.alignment: Qt.AlignHCenter
                            orientation: Qt.Vertical
                            from: 0
                            to: 100
                            value: PlayerController.volumeLevel
                            onMoved: PlayerController.setVolume(Math.round(value))
                        }

                        QQC2.ToolButton {
                            icon.name: PlayerController.volumeMuted ? "audio-volume-muted" : "audio-volume-high"
                            Layout.alignment: Qt.AlignHCenter
                            onClicked: PlayerController.toggleMute()
                        }
                    }
                }
            }
        }

    }

    } // playerBarContainer
}
