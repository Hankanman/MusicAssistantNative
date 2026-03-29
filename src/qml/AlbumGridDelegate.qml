import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

QQC2.ItemDelegate {
    id: albumCard

    signal itemActivated(string uri)
    signal playRequested(string uri)

    required property string name
    required property string artistName
    required property string imageUrl
    required property string uri
    required property int year

    // Parse "path|provider" for image URL
    readonly property var imageParts: imageUrl ? imageUrl.split("|") : []
    readonly property string resolvedImageUrl: {
        if (imageParts.length >= 2 && imageParts[0] !== "")
            return MaClient.getImageUrl(imageParts[0], imageParts[1], 300)
        return ""
    }

    implicitWidth: Kirigami.Units.gridUnit * 10
    implicitHeight: contentCol.implicitHeight + Kirigami.Units.smallSpacing * 2

    contentItem: ColumnLayout {
        id: contentCol
        spacing: Kirigami.Units.smallSpacing

        // Album art
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: width
            radius: Kirigami.Units.cornerRadius
            color: Kirigami.Theme.backgroundColor
            clip: true

            Image {
                id: coverImage
                anchors.fill: parent
                source: albumCard.resolvedImageUrl
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                visible: status === Image.Ready
            }

            Kirigami.Icon {
                anchors.centerIn: parent
                source: "media-album-cover"
                width: parent.width * 0.4
                height: width
                visible: coverImage.status !== Image.Ready
                color: Kirigami.Theme.disabledTextColor
            }

            // Play button overlay on hover
            Rectangle {
                anchors.fill: parent
                color: "#40000000"
                opacity: albumCard.hovered ? 1 : 0
                radius: Kirigami.Units.cornerRadius

                Behavior on opacity { NumberAnimation { duration: 150 } }

                Kirigami.Icon {
                    anchors.centerIn: parent
                    source: "media-playback-start"
                    width: Kirigami.Units.iconSizes.large
                    height: width
                    color: "white"
                }
            }
        }

        // Album name
        QQC2.Label {
            Layout.fillWidth: true
            text: albumCard.name
            font.bold: true
            elide: Text.ElideRight
            maximumLineCount: 2
            wrapMode: Text.WordWrap
        }

        // Artist + year
        QQC2.Label {
            Layout.fillWidth: true
            text: albumCard.year > 0 ? albumCard.artistName + " · " + albumCard.year : albumCard.artistName
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            color: Kirigami.Theme.disabledTextColor
            elide: Text.ElideRight
            maximumLineCount: 1
        }
    }

    onClicked: albumCard.itemActivated(uri)
    onDoubleClicked: albumCard.playRequested(uri)
}
