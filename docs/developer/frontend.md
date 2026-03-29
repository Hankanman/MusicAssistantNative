# QML Frontend

The UI is built with QML and [Kirigami](https://develop.kde.org/frameworks/kirigami/), KDE's convergent UI framework.

## Page Structure

All pages use Kirigami components and follow KDE HIG patterns:

| File | Component | Description |
|------|-----------|-------------|
| `Main.qml` | `Kirigami.ApplicationWindow` | App shell, sidebar, bottom player bar |
| `NowPlayingPage.qml` | `Kirigami.Page` | Album art, track info, full controls |
| `LibraryPage.qml` | `Kirigami.ScrollablePage` | Tabbed library with search |
| `MediaItemDelegate.qml` | `QQC2.ItemDelegate` | Reusable list item for media |
| `QueuePage.qml` | `Kirigami.ScrollablePage` | Queue listing and management |
| `PlayersPage.qml` | `Kirigami.ScrollablePage` | Player list with volume controls |
| `SettingsPage.qml` | `Kirigami.Page` | Server connection config |

## Main.qml

The root window provides:

- **GlobalDrawer** — sidebar navigation with icons for each page
- **PageStack** — manages page navigation
- **Footer ToolBar** — persistent bottom player bar

### Sidebar

```qml
globalDrawer: Kirigami.GlobalDrawer {
    actions: [
        Kirigami.Action { text: i18n("Now Playing"); icon.name: "media-playback-start" },
        Kirigami.Action { text: i18n("Library"); icon.name: "view-media-playlist" },
        Kirigami.Action { text: i18n("Queue"); icon.name: "amarok_playlist" },
        Kirigami.Action { text: i18n("Players"); icon.name: "speaker" },
        Kirigami.Action { text: i18n("Settings"); icon.name: "settings-configure" },
    ]
}
```

### Bottom Player Bar

Always visible when connected with a selected player. Contains album art thumbnail, track info, transport controls, and volume slider.

## Accessing C++ Objects from QML

All backend objects are exposed as context properties in `main.cpp`:

```qml
// These are available globally in all QML files:
MaClient.connected          // bool
MaClient.authenticated      // bool
MaClient.serverName         // string

PlayerController.isPlaying  // bool
PlayerController.playPause() // method

QueueController.shuffleEnabled // bool
QueueController.itemModel      // QueueItemModel

LibraryController.tracksModel  // MediaItemModel
LibraryController.search(query) // method

PlayerModel                    // QAbstractListModel
```

## Image Loading

Album art is loaded directly via QML `Image` elements using URLs built by `MaClient.getImageUrl()`:

```qml
Image {
    source: model.imageUrl ? MaClient.getImageUrl(model.imageUrl) : ""
}
```

The `getImageUrl()` method constructs the full image proxy URL (including size and authentication parameters), so no custom `QQuickAsyncImageProvider` is needed — Qt's built-in HTTP image loading handles everything.

## Kirigami Patterns Used

### PlaceholderMessage

Shown when lists are empty:

```qml
Kirigami.PlaceholderMessage {
    anchors.centerIn: parent
    visible: listView.count === 0
    text: i18n("No items")
    icon.name: "view-media-playlist"
}
```

### InlineMessage

For connection status and errors:

```qml
Kirigami.InlineMessage {
    type: Kirigami.MessageType.Positive
    text: i18n("Connected to %1", MaClient.serverName)
    visible: MaClient.authenticated
}
```

### FormLayout

For the settings page:

```qml
Kirigami.FormLayout {
    QQC2.TextField {
        Kirigami.FormData.label: i18n("Server URL:")
    }
}
```

## Internationalization

All user-visible strings use `i18n()`:

```qml
text: i18n("Now Playing")
text: i18n("%1 items", QueueController.itemCount)
text: i18n("Connected to %1 (v%2)", MaClient.serverName, MaClient.serverVersion)
```

## Adding a New Page

1. Create `src/qml/NewPage.qml` using `Kirigami.Page` or `Kirigami.ScrollablePage`
2. Add the file to `qt_add_qml_module` in `src/CMakeLists.txt`
3. Add a `Component` and navigation action in `Main.qml`
4. If it needs new data, create a controller class in C++ and expose it via `main.cpp`
