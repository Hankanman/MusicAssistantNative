import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: artistDetailPage
    title: artistName

    required property string artistName
    required property string imageUrl
    required property string itemId
    required property string provider

    // Resolve artist image
    readonly property var imageParts: imageUrl ? imageUrl.split("|") : []
    readonly property string resolvedImageUrl: {
        if (imageParts.length >= 2 && imageParts[0] !== "")
            return MaClient.getImageUrl(imageParts[0], imageParts[1], 600)
        return ""
    }

    // Albums model from signal
    ListModel {
        id: albumsListModel
    }

    Connections {
        target: LibraryController
        function onArtistAlbumsLoaded(albums) {
            albumsListModel.clear()
            for (var i = 0; i < albums.length; i++) {
                var a = albums[i]
                // Extract image URL
                var imgUrl = ""
                if (a.metadata && a.metadata.images && a.metadata.images.length > 0) {
                    var img = a.metadata.images[0]
                    if (img.path && img.provider) {
                        imgUrl = img.path + "|" + img.provider
                    }
                }
                // Extract provider mapping
                var provId = ""
                var provInstance = ""
                if (a.provider_mappings && a.provider_mappings.length > 0) {
                    provId = a.provider_mappings[0].item_id || ""
                    provInstance = a.provider_mappings[0].provider_instance || ""
                }
                albumsListModel.append({
                    albumName: a.name || "",
                    albumArtist: a.artists && a.artists.length > 0 ? a.artists[0].name || "" : "",
                    albumImageUrl: imgUrl,
                    albumYear: a.year || 0,
                    albumUri: a.uri || "",
                    albumItemId: provId,
                    albumProvider: provInstance
                })
            }
        }
    }

    Component.onCompleted: {
        LibraryController.loadArtistAlbums(itemId, provider)
    }

    ColumnLayout {
        width: parent.width
        spacing: Kirigami.Units.largeSpacing

        // Header section with artist image and name
        ColumnLayout {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.largeSpacing

            // Artist image (circular)
            Rectangle {
                Layout.preferredWidth: Kirigami.Units.gridUnit * 10
                Layout.preferredHeight: Kirigami.Units.gridUnit * 10
                Layout.alignment: Qt.AlignHCenter
                radius: width / 2
                color: Kirigami.Theme.backgroundColor
                clip: true

                Image {
                    id: artistImage
                    anchors.fill: parent
                    source: artistDetailPage.resolvedImageUrl
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                    visible: status === Image.Ready
                }

                Kirigami.Icon {
                    anchors.centerIn: parent
                    source: "view-media-artist"
                    width: parent.width * 0.4
                    height: width
                    visible: artistImage.status !== Image.Ready
                    color: Kirigami.Theme.disabledTextColor
                }
            }

            // Artist name
            QQC2.Label {
                text: artistDetailPage.artistName
                font.bold: true
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.8
                Layout.alignment: Qt.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            QQC2.Label {
                text: i18n("%1 albums", albumsListModel.count)
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                color: Kirigami.Theme.disabledTextColor
                Layout.alignment: Qt.AlignHCenter
                visible: albumsListModel.count > 0
            }
        }

        // Separator
        Kirigami.Separator {
            Layout.fillWidth: true
        }

        // Section header
        Kirigami.Heading {
            text: i18n("Albums")
            level: 3
            Layout.leftMargin: Kirigami.Units.largeSpacing
            visible: albumsListModel.count > 0
        }

        // Albums grid
        GridLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            columns: Math.max(1, Math.floor(parent.width / (Kirigami.Units.gridUnit * 11)))
            columnSpacing: Kirigami.Units.smallSpacing
            rowSpacing: Kirigami.Units.smallSpacing

            Repeater {
                model: albumsListModel
                delegate: AlbumGridDelegate {
                    Layout.fillWidth: true
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 10
                    Layout.maximumWidth: Kirigami.Units.gridUnit * 12
                    name: model.albumName
                    artistName: model.albumArtist
                    imageUrl: model.albumImageUrl
                    uri: model.albumUri
                    year: model.albumYear
                    onItemActivated: {
                        applicationWindow().pageStack.push(Qt.resolvedUrl("AlbumDetailPage.qml"), {
                            albumName: model.albumName,
                            artistName: model.albumArtist,
                            imageUrl: model.albumImageUrl,
                            year: model.albumYear,
                            itemId: model.albumItemId,
                            provider: model.albumProvider,
                            uri: model.albumUri
                        })
                    }
                    onPlayRequested: (uri) => {
                        if (uri) QueueController.playMedia(uri, "replace")
                    }
                }
            }
        }

        // Loading indicator
        QQC2.BusyIndicator {
            Layout.alignment: Qt.AlignCenter
            running: albumsListModel.count === 0 && LibraryController.loading
            visible: running
        }

        // Empty state
        Kirigami.PlaceholderMessage {
            Layout.fillWidth: true
            visible: albumsListModel.count === 0 && !LibraryController.loading
            text: i18n("No albums found")
            icon.name: "media-album-cover"
        }
    }
}
