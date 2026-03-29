import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

QQC2.ItemDelegate {
    id: delegate

    signal itemActivated(string uri)
    signal detailRequested(var itemData)

    // Parse "path|provider" into a usable image URL
    readonly property var imageParts: model.imageUrl ? model.imageUrl.split("|") : []
    readonly property string resolvedImageUrl: {
        if (imageParts.length >= 2 && imageParts[0] !== "") {
            return MaClient.getImageUrl(imageParts[0], imageParts[1], 300)
        }
        return ""
    }

    contentItem: RowLayout {
        spacing: Kirigami.Units.smallSpacing

        // Thumbnail with fallback
        Item {
            Layout.preferredWidth: Kirigami.Units.gridUnit * 3
            Layout.preferredHeight: Kirigami.Units.gridUnit * 3

            Image {
                id: thumbImage
                anchors.fill: parent
                source: delegate.resolvedImageUrl
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                cache: true
                visible: status === Image.Ready

                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    border.color: Kirigami.Theme.disabledTextColor
                    border.width: 1
                    radius: 2
                }
            }

            // Fallback icon — shown when no URL, loading, or load failed
            Kirigami.Icon {
                anchors.fill: parent
                visible: thumbImage.status !== Image.Ready
                color: Kirigami.Theme.disabledTextColor
                source: {
                    switch (model.mediaType) {
                    case "artist": return "view-media-artist"
                    case "album": return "media-album-cover"
                    case "track": return "audio-x-generic"
                    case "playlist": return "view-media-playlist"
                    case "radio": return "radio"
                    default: return "audio-x-generic"
                    }
                }
            }
        }

        // Info
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0

            QQC2.Label {
                text: model.name || ""
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            QQC2.Label {
                text: {
                    var parts = []
                    if (model.artistName) parts.push(model.artistName)
                    if (model.albumName) parts.push(model.albumName)
                    return parts.join(" - ")
                }
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                color: Kirigami.Theme.disabledTextColor
                elide: Text.ElideRight
                Layout.fillWidth: true
                visible: text !== ""
            }
        }

        // Duration
        QQC2.Label {
            text: model.duration > 0 ? formatTime(model.duration) : ""
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            color: Kirigami.Theme.disabledTextColor
            visible: text !== ""
        }

        // Favorite indicator
        Kirigami.Icon {
            source: "emblem-favorite"
            implicitWidth: Kirigami.Units.iconSizes.small
            implicitHeight: Kirigami.Units.iconSizes.small
            visible: model.favorite === true
            color: Kirigami.Theme.positiveTextColor
        }

        // Play button
        QQC2.ToolButton {
            icon.name: "media-playback-start"
            onClicked: {
                var uri = model.uri || ""
                if (uri !== "") {
                    delegate.itemActivated(uri)
                }
            }
            QQC2.ToolTip.text: i18n("Play")
            QQC2.ToolTip.visible: hovered
        }
    }

    onClicked: {
        var mediaType = model.mediaType || ""
        if (mediaType === "artist" || mediaType === "album" || mediaType === "playlist") {
            delegate.detailRequested({
                mediaType: mediaType,
                name: model.name || "",
                artistName: model.artistName || "",
                albumName: model.albumName || "",
                imageUrl: model.imageUrl || "",
                uri: model.uri || "",
                itemId: model.itemId || "",
                provider: model.provider || "",
                year: model.year || 0
            })
        } else {
            var uri = model.uri || ""
            if (uri !== "") {
                delegate.itemActivated(uri)
            }
        }
    }

    function formatTime(seconds) {
        if (seconds <= 0) return ""
        var mins = Math.floor(seconds / 60)
        var secs = Math.floor(seconds % 60)
        return mins + ":" + (secs < 10 ? "0" : "") + secs
    }
}
