import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: nowPlayingPage
    title: i18n("Now Playing")

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        // Album art
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: parent.height * 0.5

            Image {
                id: albumArt
                anchors.centerIn: parent
                width: Math.min(parent.width, parent.height)
                height: width
                source: PlayerController.currentTrackImageUrl !== ""
                    ? "image://ma/" + PlayerController.currentTrackImageUrl
                    : ""
                fillMode: Image.PreserveAspectCrop
                visible: source !== ""

                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    border.color: Kirigami.Theme.disabledTextColor
                    border.width: 1
                    radius: 8
                }
            }

            Kirigami.Icon {
                anchors.centerIn: parent
                source: "multimedia-player"
                width: Kirigami.Units.gridUnit * 8
                height: width
                visible: albumArt.source === ""
                color: Kirigami.Theme.disabledTextColor
            }
        }

        // Track info
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: PlayerController.currentTrackTitle || i18n("Nothing playing")
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.5
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            QQC2.Label {
                text: PlayerController.currentTrackArtist
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.1
                horizontalAlignment: Text.AlignHCenter
                color: Kirigami.Theme.disabledTextColor
                elide: Text.ElideRight
                Layout.fillWidth: true
                visible: text !== ""
            }
            QQC2.Label {
                text: PlayerController.currentTrackAlbum
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                horizontalAlignment: Text.AlignHCenter
                color: Kirigami.Theme.disabledTextColor
                elide: Text.ElideRight
                Layout.fillWidth: true
                visible: text !== ""
            }
        }

        // Progress bar
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            QQC2.Slider {
                id: seekSlider
                Layout.fillWidth: true
                from: 0
                to: PlayerController.duration > 0 ? PlayerController.duration : 1
                value: PlayerController.elapsed
                enabled: PlayerController.duration > 0

                onMoved: PlayerController.seek(Math.round(value))
            }

            RowLayout {
                Layout.fillWidth: true

                QQC2.Label {
                    text: formatTime(PlayerController.elapsed)
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    color: Kirigami.Theme.disabledTextColor
                }
                Item { Layout.fillWidth: true }
                QQC2.Label {
                    text: formatTime(PlayerController.duration)
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    color: Kirigami.Theme.disabledTextColor
                }
            }
        }

        // Playback controls
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.largeSpacing

            QQC2.ToolButton {
                icon.name: "media-playlist-shuffle"
                checked: QueueController.shuffleEnabled
                onClicked: QueueController.setShuffle(!QueueController.shuffleEnabled)
                QQC2.ToolTip.text: i18n("Shuffle")
                QQC2.ToolTip.visible: hovered
            }
            QQC2.ToolButton {
                icon.name: "media-skip-backward"
                icon.width: Kirigami.Units.iconSizes.medium
                icon.height: Kirigami.Units.iconSizes.medium
                onClicked: PlayerController.previous()
            }
            QQC2.ToolButton {
                icon.name: PlayerController.isPlaying ? "media-playback-pause" : "media-playback-start"
                icon.width: Kirigami.Units.iconSizes.large
                icon.height: Kirigami.Units.iconSizes.large
                onClicked: PlayerController.playPause()
            }
            QQC2.ToolButton {
                icon.name: "media-skip-forward"
                icon.width: Kirigami.Units.iconSizes.medium
                icon.height: Kirigami.Units.iconSizes.medium
                onClicked: PlayerController.next()
            }
            QQC2.ToolButton {
                icon.name: {
                    switch (QueueController.repeatMode) {
                    case "one": return "media-playlist-repeat-song"
                    case "all": return "media-playlist-repeat"
                    default: return "media-playlist-no-repeat"
                    }
                }
                onClicked: {
                    switch (QueueController.repeatMode) {
                    case "off": QueueController.setRepeat("all"); break
                    case "all": QueueController.setRepeat("one"); break
                    case "one": QueueController.setRepeat("off"); break
                    }
                }
                QQC2.ToolTip.text: i18n("Repeat: %1", QueueController.repeatMode)
                QQC2.ToolTip.visible: hovered
            }
        }
    }

    function formatTime(seconds) {
        if (seconds <= 0) return "0:00"
        var mins = Math.floor(seconds / 60)
        var secs = Math.floor(seconds % 60)
        return mins + ":" + (secs < 10 ? "0" : "") + secs
    }
}
