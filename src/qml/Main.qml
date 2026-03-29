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

    pageStack.initialPage: settingsPage

    function switchPage(page) {
        if (pageStack.currentItem !== page) {
            pageStack.clear()
            pageStack.push(page)
        }
    }

    // Give pageStack bottom margin so content isn't hidden behind player bar
    pageStack.anchors.bottomMargin: root.showPlayerBar ? playerBar.height : 0

    // Persistent bottom player bar — anchored to window contentItem (below sidebar)
    QQC2.ToolBar {
        id: playerBar
        visible: root.showPlayerBar
        z: 1

        parent: root.contentItem
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Kirigami.Units.smallSpacing
            anchors.rightMargin: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.smallSpacing

            // Album art thumbnail
            Image {
                source: PlayerController.currentTrackImageUrl !== ""
                    ? "image://ma/" + PlayerController.currentTrackImageUrl
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

            // Volume
            QQC2.ToolButton {
                icon.name: PlayerController.volumeMuted ? "audio-volume-muted" : "audio-volume-high"
                onClicked: PlayerController.toggleMute()
                QQC2.ToolTip.text: i18n("Mute")
                QQC2.ToolTip.visible: hovered
            }
            QQC2.Slider {
                Layout.preferredWidth: Kirigami.Units.gridUnit * 6
                from: 0
                to: 100
                value: PlayerController.volumeLevel
                onMoved: PlayerController.setVolume(Math.round(value))
            }
        }
    }
}
