import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: playlistDetailPage
    title: playlistName

    required property string playlistName
    required property string imageUrl
    required property string itemId
    required property string provider
    property string uri: ""

    // Resolve playlist image
    readonly property var imageParts: imageUrl ? imageUrl.split("|") : []
    readonly property string resolvedImageUrl: {
        if (imageParts.length >= 2 && imageParts[0] !== "")
            return MaClient.getImageUrl(imageParts[0], imageParts[1], 600)
        return ""
    }

    actions: [
        Kirigami.Action {
            text: i18n("Play All")
            icon.name: "media-playback-start"
            onTriggered: {
                if (playlistDetailPage.uri !== "") {
                    QueueController.playMedia(playlistDetailPage.uri, "replace")
                }
            }
        }
    ]

    // Track list model from signal
    ListModel {
        id: trackListModel
    }

    Connections {
        target: LibraryController
        function onPlaylistTracksLoaded(tracks) {
            trackListModel.clear()
            for (var i = 0; i < tracks.length; i++) {
                var t = tracks[i]
                // Extract artist name
                var artist = ""
                if (t.artists && t.artists.length > 0) {
                    artist = t.artists[0].name || ""
                }
                trackListModel.append({
                    trackName: t.name || "",
                    trackNumber: i + 1,
                    trackArtist: artist,
                    trackDuration: t.duration || 0,
                    trackUri: t.uri || ""
                })
            }
        }
    }

    Component.onCompleted: {
        LibraryController.loadPlaylistTracks(itemId, provider)
    }

    ColumnLayout {
        width: parent.width
        spacing: Kirigami.Units.largeSpacing

        // Header section
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            spacing: Kirigami.Units.largeSpacing

            // Playlist image
            Rectangle {
                Layout.preferredWidth: Kirigami.Units.gridUnit * 10
                Layout.preferredHeight: Kirigami.Units.gridUnit * 10
                radius: Kirigami.Units.cornerRadius
                color: Kirigami.Theme.backgroundColor
                clip: true

                Image {
                    id: playlistImage
                    anchors.fill: parent
                    source: playlistDetailPage.resolvedImageUrl
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                    visible: status === Image.Ready
                }

                Kirigami.Icon {
                    anchors.centerIn: parent
                    source: "view-media-playlist"
                    width: parent.width * 0.4
                    height: width
                    visible: playlistImage.status !== Image.Ready
                    color: Kirigami.Theme.disabledTextColor
                }
            }

            // Playlist info
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.Label {
                    text: playlistDetailPage.playlistName
                    font.bold: true
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.5
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                QQC2.Label {
                    text: i18n("%1 tracks", trackListModel.count)
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    color: Kirigami.Theme.disabledTextColor
                    visible: trackListModel.count > 0
                }
            }
        }

        // Separator
        Kirigami.Separator {
            Layout.fillWidth: true
        }

        // Track list
        Repeater {
            model: trackListModel
            delegate: QQC2.ItemDelegate {
                width: parent.width
                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    // Track number
                    QQC2.Label {
                        text: String(model.trackNumber)
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                        horizontalAlignment: Text.AlignRight
                        color: Kirigami.Theme.disabledTextColor
                    }

                    // Track info
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        QQC2.Label {
                            text: model.trackName
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        QQC2.Label {
                            text: model.trackArtist
                            font.pointSize: Kirigami.Theme.smallFont.pointSize
                            color: Kirigami.Theme.disabledTextColor
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                            visible: text !== ""
                        }
                    }

                    // Duration
                    QQC2.Label {
                        text: model.trackDuration > 0 ? formatTime(model.trackDuration) : ""
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                        color: Kirigami.Theme.disabledTextColor
                        visible: text !== ""
                    }

                    // Play button
                    QQC2.ToolButton {
                        icon.name: "media-playback-start"
                        onClicked: {
                            if (model.trackUri !== "") {
                                QueueController.playMedia(model.trackUri, "replace")
                            }
                        }
                        QQC2.ToolTip.text: i18n("Play")
                        QQC2.ToolTip.visible: hovered
                    }
                }

                onClicked: {
                    if (model.trackUri !== "") {
                        QueueController.playMedia(model.trackUri, "replace")
                    }
                }
            }
        }

        // Loading indicator
        QQC2.BusyIndicator {
            Layout.alignment: Qt.AlignCenter
            running: trackListModel.count === 0 && LibraryController.loading
            visible: running
        }

        // Empty state
        Kirigami.PlaceholderMessage {
            Layout.fillWidth: true
            visible: trackListModel.count === 0 && !LibraryController.loading
            text: i18n("No tracks found")
            icon.name: "audio-x-generic"
        }
    }

    function formatTime(seconds) {
        if (seconds <= 0) return ""
        var mins = Math.floor(seconds / 60)
        var secs = Math.floor(seconds % 60)
        return mins + ":" + (secs < 10 ? "0" : "") + secs
    }
}
