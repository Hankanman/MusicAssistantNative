import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: libraryPage
    title: i18n("Library")
    padding: 0

    property int currentTab: 0
    property bool gridMode: false
    property string searchQuery: ""

    // Tab definitions
    readonly property var tabs: [
        { name: i18n("Artists"),   type: "artists",   icon: "view-media-artist" },
        { name: i18n("Albums"),    type: "albums",    icon: "media-album-cover" },
        { name: i18n("Tracks"),    type: "tracks",    icon: "audio-x-generic" },
        { name: i18n("Playlists"), type: "playlists", icon: "view-media-playlist" },
        { name: i18n("Radios"),    type: "radios",    icon: "radio" },
    ]

    actions: [
        Kirigami.Action {
            icon.name: libraryPage.gridMode ? "view-list-details" : "view-list-icons"
            text: libraryPage.gridMode ? i18n("List View") : i18n("Grid View")
            visible: currentTab === 1 // Only for Albums
            onTriggered: libraryPage.gridMode = !libraryPage.gridMode
        }
    ]

    header: ColumnLayout {
        spacing: 0

        Kirigami.SearchField {
            id: searchField
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            placeholderText: i18n("Search music...")
            onAccepted: {
                if (text.length > 0) {
                    libraryPage.searchQuery = text
                    LibraryController.search(text)
                } else {
                    libraryPage.searchQuery = ""
                }
            }
        }
    }

    // Main content
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Content area
        QQC2.StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: searchQuery !== "" ? 5 : currentTab

            // Tab 0: Artists (list)
            ListView {
                model: LibraryController.artistsModel
                delegate: MediaItemDelegate {
                    width: ListView.view.width
                    onItemActivated: (uri) => { if (uri) QueueController.playMedia(uri, "replace") }
                }
                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    visible: parent.count === 0 && !LibraryController.loading
                    text: i18n("No artists"); icon.name: "view-media-artist"
                }
            }

            // Tab 1: Albums (list or grid)
            Loader {
                active: true
                sourceComponent: libraryPage.gridMode ? albumGridComponent : albumListComponent
            }

            // Tab 2: Tracks (list)
            ListView {
                model: LibraryController.tracksModel
                delegate: MediaItemDelegate {
                    width: ListView.view.width
                    onItemActivated: (uri) => { if (uri) QueueController.playMedia(uri, "replace") }
                }
                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    visible: parent.count === 0 && !LibraryController.loading
                    text: i18n("No tracks"); icon.name: "audio-x-generic"
                }
            }

            // Tab 3: Playlists (list)
            ListView {
                model: LibraryController.playlistsModel
                delegate: MediaItemDelegate {
                    width: ListView.view.width
                    onItemActivated: (uri) => { if (uri) QueueController.playMedia(uri, "replace") }
                }
                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    visible: parent.count === 0 && !LibraryController.loading
                    text: i18n("No playlists"); icon.name: "view-media-playlist"
                }
            }

            // Tab 4: Radios (list)
            ListView {
                model: LibraryController.radiosModel
                delegate: MediaItemDelegate {
                    width: ListView.view.width
                    onItemActivated: (uri) => { if (uri) QueueController.playMedia(uri, "replace") }
                }
                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    visible: parent.count === 0 && !LibraryController.loading
                    text: i18n("No radios"); icon.name: "radio"
                }
            }

            // Tab 5: Search results
            ListView {
                model: LibraryController.searchResultsModel
                delegate: MediaItemDelegate {
                    width: ListView.view.width
                    onItemActivated: (uri) => { if (uri) QueueController.playMedia(uri, "replace") }
                }
                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    visible: parent.count === 0 && !LibraryController.loading
                    text: i18n("No results"); icon.name: "search"
                }
            }
        }

        // Loading indicator
        QQC2.BusyIndicator {
            Layout.alignment: Qt.AlignCenter
            running: LibraryController.loading
            visible: running
        }
    }

    // Navigation tab bar at footer
    footer: Kirigami.NavigationTabBar {
        visible: searchQuery === ""
        actions: [
            Kirigami.Action {
                text: tabs[0].name
                icon.name: tabs[0].icon
                checked: currentTab === 0
                onTriggered: { currentTab = 0; loadTab(0) }
            },
            Kirigami.Action {
                text: tabs[1].name
                icon.name: tabs[1].icon
                checked: currentTab === 1
                onTriggered: { currentTab = 1; loadTab(1) }
            },
            Kirigami.Action {
                text: tabs[2].name
                icon.name: tabs[2].icon
                checked: currentTab === 2
                onTriggered: { currentTab = 2; loadTab(2) }
            },
            Kirigami.Action {
                text: tabs[3].name
                icon.name: tabs[3].icon
                checked: currentTab === 3
                onTriggered: { currentTab = 3; loadTab(3) }
            },
            Kirigami.Action {
                text: tabs[4].name
                icon.name: tabs[4].icon
                checked: currentTab === 4
                onTriggered: { currentTab = 4; loadTab(4) }
            }
        ]
    }

    // Album list view component
    Component {
        id: albumListComponent
        ListView {
            model: LibraryController.albumsModel
            delegate: MediaItemDelegate {
                width: ListView.view.width
                onItemActivated: (uri) => { if (uri) QueueController.playMedia(uri, "replace") }
            }
            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                visible: parent.count === 0 && !LibraryController.loading
                text: i18n("No albums"); icon.name: "media-album-cover"
            }
        }
    }

    // Album grid view component
    Component {
        id: albumGridComponent
        GridView {
            id: albumGrid
            cellWidth: Kirigami.Units.gridUnit * 11
            cellHeight: Kirigami.Units.gridUnit * 14
            model: LibraryController.albumsModel
            delegate: AlbumGridDelegate {
                width: albumGrid.cellWidth - Kirigami.Units.smallSpacing * 2
                name: model.name || ""
                artistName: model.artistName || ""
                imageUrl: model.imageUrl || ""
                uri: model.uri || ""
                year: model.year || 0
                onItemActivated: (uri) => { if (uri) QueueController.playMedia(uri, "replace") }
                onPlayRequested: (uri) => { if (uri) QueueController.playMedia(uri, "replace") }
            }
            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                visible: albumGrid.count === 0 && !LibraryController.loading
                text: i18n("No albums"); icon.name: "media-album-cover"
            }
        }
    }

    function loadTab(index) {
        if (index < tabs.length) {
            LibraryController.loadLibrary(tabs[index].type)
        }
    }

    onVisibleChanged: {
        if (visible && MaClient.authenticated) {
            loadTab(currentTab)
        }
    }

    Connections {
        target: MaClient
        function onAuthenticatedChanged() {
            if (MaClient.authenticated && visible) {
                loadTab(currentTab)
            }
        }
    }
}
