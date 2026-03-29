import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

QQC2.ItemDelegate {
    id: delegate

    signal itemActivated(string uri)

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

        // Thumbnail
        Image {
            source: delegate.resolvedImageUrl
            Layout.preferredWidth: Kirigami.Units.gridUnit * 3
            Layout.preferredHeight: Kirigami.Units.gridUnit * 3
            fillMode: Image.PreserveAspectCrop
            visible: delegate.resolvedImageUrl !== ""
            asynchronous: true
            cache: true

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: Kirigami.Theme.disabledTextColor
                border.width: 1
                radius: 2
            }
        }

        Kirigami.Icon {
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
            Layout.preferredWidth: Kirigami.Units.gridUnit * 3
            Layout.preferredHeight: Kirigami.Units.gridUnit * 3
            visible: delegate.resolvedImageUrl === ""
            color: Kirigami.Theme.disabledTextColor
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
    }

    onClicked: {
        delegate.itemActivated(model.uri || "")
    }

    function formatTime(seconds) {
        if (seconds <= 0) return ""
        var mins = Math.floor(seconds / 60)
        var secs = Math.floor(seconds % 60)
        return mins + ":" + (secs < 10 ? "0" : "") + secs
    }
}
