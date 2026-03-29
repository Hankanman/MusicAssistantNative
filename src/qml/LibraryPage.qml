import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: libraryPage
    title: i18n("Library")

    supportsRefreshing: true
    onRefreshingChanged: {
        if (refreshing) {
            loadCurrentType()
            refreshing = false
        }
    }

    header: ColumnLayout {
        spacing: 0

        // Search bar
        Kirigami.SearchField {
            id: searchField
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            placeholderText: i18n("Search music...")
            onAccepted: {
                if (text.length > 0) {
                    LibraryController.search(text)
                    tabBar.currentIndex = 5  // search results tab
                }
            }
        }

        // Type tabs
        QQC2.TabBar {
            id: tabBar
            Layout.fillWidth: true

            QQC2.TabButton { text: i18n("Artists") }
            QQC2.TabButton { text: i18n("Albums") }
            QQC2.TabButton { text: i18n("Tracks") }
            QQC2.TabButton { text: i18n("Playlists") }
            QQC2.TabButton { text: i18n("Radios") }
            QQC2.TabButton { text: i18n("Search"); visible: searchField.text.length > 0 }

            onCurrentIndexChanged: {
                if (currentIndex < 5) {
                    loadCurrentType()
                }
            }
        }
    }

    ListView {
        id: listView
        model: getCurrentModel()
        delegate: MediaItemDelegate {
            width: listView.width
            onItemActivated: (uri) => {
                if (uri !== "") {
                    QueueController.playMedia(uri, "replace")
                }
            }
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            visible: listView.count === 0 && !LibraryController.loading
            text: i18n("No items")
            explanation: tabBar.currentIndex === 5
                ? i18n("No results for your search")
                : i18n("Your library is empty for this category")
            icon.name: "view-media-playlist"
        }

        QQC2.BusyIndicator {
            anchors.centerIn: parent
            running: LibraryController.loading
            visible: running
        }
    }

    // Load data when page becomes visible (not on construction, since
    // pages are pre-created before auth completes)
    onVisibleChanged: {
        if (visible && MaClient.authenticated && getCurrentModel().count === 0) {
            loadCurrentType()
        }
    }

    Connections {
        target: MaClient
        function onAuthenticatedChanged() {
            if (MaClient.authenticated && visible) {
                loadCurrentType()
            }
        }
    }

    function getCurrentModel() {
        switch (tabBar.currentIndex) {
        case 0: return LibraryController.artistsModel
        case 1: return LibraryController.albumsModel
        case 2: return LibraryController.tracksModel
        case 3: return LibraryController.playlistsModel
        case 4: return LibraryController.radiosModel
        case 5: return LibraryController.searchResultsModel
        default: return LibraryController.artistsModel
        }
    }

    function loadCurrentType() {
        var types = ["artists", "albums", "tracks", "playlists", "radios"]
        if (tabBar.currentIndex < types.length) {
            LibraryController.loadLibrary(types[tabBar.currentIndex])
        }
    }
}
