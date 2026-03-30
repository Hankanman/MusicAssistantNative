import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: albumDetailPage
    title: albumName

    required property string albumName
    required property string artistName
    required property string imageUrl
    required property int year
    required property string itemId
    required property string provider
    property string uri: ""

    // Resolve album art
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
                if (albumDetailPage.uri !== "") {
                    QueueController.playMedia(albumDetailPage.uri, "replace")
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
        function onAlbumTracksLoaded(tracks) {
            trackListModel.clear()
            for (var i = 0; i < tracks.length; i++) {
                var t = tracks[i]
                trackListModel.append({
                    trackName: t.name || "",
                    trackNumber: t.track_number || (i + 1),
                    trackDuration: t.duration || 0,
                    trackUri: t.uri || ""
                })
            }
        }
    }

    Component.onCompleted: {
        LibraryController.loadAlbumTracks(itemId, provider)
    }

    ColumnLayout {
        width: parent.width
        spacing: Kirigami.Units.largeSpacing

        // Header section with album art and info
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            spacing: Kirigami.Units.largeSpacing

            // Album art
            Rectangle {
                Layout.preferredWidth: Kirigami.Units.gridUnit * 12
                Layout.preferredHeight: Kirigami.Units.gridUnit * 12
                radius: Kirigami.Units.cornerRadius
                color: Kirigami.Theme.backgroundColor
                clip: true

                Image {
                    id: albumArt
                    anchors.fill: parent
                    source: albumDetailPage.resolvedImageUrl
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                    visible: status === Image.Ready
                }

                Kirigami.Icon {
                    anchors.centerIn: parent
                    source: "media-album-cover"
                    width: parent.width * 0.4
                    height: width
                    visible: albumArt.status !== Image.Ready
                    color: Kirigami.Theme.disabledTextColor
                }
            }

            // Album info
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.Label {
                    text: albumDetailPage.albumName
                    font.bold: true
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.5
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                QQC2.Label {
                    text: albumDetailPage.artistName
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.1
                    color: Kirigami.Theme.disabledTextColor
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    visible: text !== ""
                }
                QQC2.Label {
                    text: albumDetailPage.year > 0 ? String(albumDetailPage.year) : ""
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    color: Kirigami.Theme.disabledTextColor
                    visible: text !== ""
                }
                QQC2.Label {
                    text: i18n("%1 tracks", trackListModel.count)
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    color: Kirigami.Theme.disabledTextColor
                    visible: trackListModel.count > 0
                }
            }
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

                    // Track title
                    QQC2.Label {
                        text: model.trackName
                        elide: Text.ElideRight
                        Layout.fillWidth: true
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
            running: trackListModel.count === 0
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
