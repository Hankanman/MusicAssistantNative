import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Effects
import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: nowPlayingPage
    title: i18n("Now Playing")
    padding: 0

    // Blurred background album art
    Image {
        id: bgImage
        anchors.fill: parent
        source: PlayerController.currentTrackImageUrl || ""
        fillMode: Image.PreserveAspectCrop
        visible: false
        asynchronous: true
    }

    MultiEffect {
        source: bgImage
        anchors.fill: parent
        blurEnabled: true
        blurMax: 64
        blur: 1.0
        opacity: bgImage.status === Image.Ready ? 0.3 : 0

        Behavior on opacity { NumberAnimation { duration: 500 } }
    }

    // Dark overlay for readability
    Rectangle {
        anchors.fill: parent
        color: Kirigami.Theme.backgroundColor
        opacity: 0.7
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing * 2
        spacing: Kirigami.Units.largeSpacing

        Item { Layout.fillHeight: true; Layout.maximumHeight: Kirigami.Units.gridUnit * 2 }

        // Album art with rounded corners and shadow
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumHeight: Kirigami.Units.gridUnit * 22
            Layout.maximumWidth: Kirigami.Units.gridUnit * 22
            Layout.alignment: Qt.AlignHCenter

            // Shadow
            Rectangle {
                anchors.fill: albumArtContainer
                anchors.margins: -2
                radius: Kirigami.Units.cornerRadius + 2
                color: "transparent"
                border.color: Qt.rgba(0, 0, 0, 0.2)
                border.width: 2
            }

            Rectangle {
                id: albumArtContainer
                anchors.centerIn: parent
                width: Math.min(parent.width, parent.height)
                height: width
                radius: Kirigami.Units.cornerRadius
                color: Kirigami.Theme.backgroundColor
                clip: true

                Image {
                    id: albumArt
                    anchors.fill: parent
                    source: PlayerController.currentTrackImageUrl || ""
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                    visible: status === Image.Ready

                    Behavior on source {
                        SequentialAnimation {
                            NumberAnimation { target: albumArt; property: "opacity"; to: 0; duration: 200 }
                            PropertyAction { target: albumArt; property: "source" }
                            NumberAnimation { target: albumArt; property: "opacity"; to: 1; duration: 300 }
                        }
                    }
                }

                Kirigami.Icon {
                    anchors.centerIn: parent
                    source: "multimedia-player"
                    width: parent.width * 0.35
                    height: width
                    visible: albumArt.status !== Image.Ready
                    color: Kirigami.Theme.disabledTextColor
                }
            }
        }

        // Track info
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: PlayerController.currentTrackTitle || i18n("Nothing playing")
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.6
                font.weight: Font.Bold
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            QQC2.Label {
                text: PlayerController.currentTrackArtist
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.15
                horizontalAlignment: Text.AlignHCenter
                opacity: 0.7
                elide: Text.ElideRight
                Layout.fillWidth: true
                visible: text !== ""
            }
            QQC2.Label {
                text: PlayerController.currentTrackAlbum
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                horizontalAlignment: Text.AlignHCenter
                opacity: 0.5
                elide: Text.ElideRight
                Layout.fillWidth: true
                visible: text !== ""
            }
        }

        // Seek bar
        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.gridUnit
            Layout.rightMargin: Kirigami.Units.gridUnit
            spacing: 0

            QQC2.Slider {
                id: seekSlider
                Layout.fillWidth: true
                from: 0
                to: PlayerController.duration > 0 ? PlayerController.duration : 1
                value: pressed ? value : PlayerController.elapsed
                enabled: PlayerController.duration > 0
                onMoved: PlayerController.seek(Math.round(value))
            }

            RowLayout {
                Layout.fillWidth: true

                QQC2.Label {
                    text: formatTime(PlayerController.elapsed)
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    opacity: 0.6
                }
                Item { Layout.fillWidth: true }
                QQC2.Label {
                    text: formatTime(PlayerController.duration)
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    opacity: 0.6
                }
            }
        }

        // Transport controls
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.gridUnit

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

            // Large play/pause button
            QQC2.RoundButton {
                implicitWidth: Kirigami.Units.gridUnit * 4
                implicitHeight: Kirigami.Units.gridUnit * 4
                icon.name: PlayerController.isPlaying ? "media-playback-pause" : "media-playback-start"
                icon.width: Kirigami.Units.iconSizes.large
                icon.height: Kirigami.Units.iconSizes.large
                onClicked: PlayerController.playPause()
                highlighted: true
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
                    case "one": return "media-repeat-single"
                    case "all": return "media-repeat-all"
                    default: return "media-repeat-none"
                    }
                }
                checked: QueueController.repeatMode !== "off"
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

        Item { Layout.fillHeight: true; Layout.maximumHeight: Kirigami.Units.gridUnit * 2 }
    }

    function formatTime(seconds) {
        if (seconds <= 0) return "0:00"
        var mins = Math.floor(seconds / 60)
        var secs = Math.floor(seconds % 60)
        return mins + ":" + (secs < 10 ? "0" : "") + secs
    }
}
