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

    globalDrawer: Kirigami.GlobalDrawer {
        title: i18n("Music Assistant")
        titleIcon: "multimedia-player"
        isMenu: false
        modal: Kirigami.Settings.isMobile
        collapsible: true
        collapsed: false

        actions: [
            Kirigami.Action {
                text: i18n("Now Playing")
                icon.name: "media-playback-start"
                onTriggered: {
                    while (pageStack.depth > 1) pageStack.pop()
                    pageStack.replace(nowPlayingComponent)
                }
            },
            Kirigami.Action {
                text: i18n("Library")
                icon.name: "view-media-playlist"
                onTriggered: {
                    while (pageStack.depth > 1) pageStack.pop()
                    pageStack.replace(libraryComponent)
                }
            },
            Kirigami.Action {
                text: i18n("Queue")
                icon.name: "amarok_playlist"
                onTriggered: {
                    while (pageStack.depth > 1) pageStack.pop()
                    pageStack.replace(queueComponent)
                }
            },
            Kirigami.Action {
                text: i18n("Players")
                icon.name: "speaker"
                onTriggered: {
                    while (pageStack.depth > 1) pageStack.pop()
                    pageStack.replace(playersComponent)
                }
            },
            Kirigami.Action {
                separator: true
            },
            Kirigami.Action {
                text: i18n("Settings")
                icon.name: "settings-configure"
                onTriggered: {
                    while (pageStack.depth > 1) pageStack.pop()
                    pageStack.replace(settingsComponent)
                }
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
                text: root.isConnected ? MaClient.serverName : i18n("Not connected")
                Layout.alignment: Qt.AlignHCenter
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                color: root.isConnected ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
            }
        }
    }

    pageStack.initialPage: settingsComponent

    // Bottom player bar
    footer: QQC2.ToolBar {
        visible: root.isConnected && PlayerController.currentPlayerId !== ""
        height: visible ? implicitHeight : 0

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
            Kirigami.Icon {
                source: PlayerController.volumeMuted ? "audio-volume-muted" : "audio-volume-high"
                implicitWidth: Kirigami.Units.iconSizes.small
                implicitHeight: Kirigami.Units.iconSizes.small
                MouseArea {
                    anchors.fill: parent
                    onClicked: PlayerController.toggleMute()
                }
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

    Component {
        id: nowPlayingComponent
        NowPlayingPage {}
    }

    Component {
        id: libraryComponent
        LibraryPage {}
    }

    Component {
        id: queueComponent
        QueuePage {}
    }

    Component {
        id: playersComponent
        PlayersPage {}
    }

    Component {
        id: settingsComponent
        SettingsPage {}
    }
}
