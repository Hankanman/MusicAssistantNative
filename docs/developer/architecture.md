# Architecture

## Overview

Music Assistant Native is a native Qt6/KDE Frameworks 6 application that acts as a remote control for a Music Assistant server. All audio processing and streaming happens server-side — the app communicates over WebSocket.

```mermaid
graph TB
    subgraph "Music Assistant Native (Desktop App)"
        QML["QML / Kirigami UI"]
        CPP["C++ Backend"]
        IMG["Image Provider"]
    end

    subgraph "Music Assistant Server"
        WS["WebSocket API (/ws)"]
        IMGP["Image Proxy (/imageproxy)"]
        LIB["Music Library"]
        PLAY["Player Manager"]
    end

    subgraph "External"
        SPEAKERS["Speakers (Sonos, AirPlay, etc.)"]
        PROVIDERS["Music Providers (Spotify, Tidal, etc.)"]
    end

    QML --> CPP
    CPP -->|"JSON-RPC over WS"| WS
    IMG -->|"HTTP + JWT"| IMGP
    WS --> LIB
    WS --> PLAY
    LIB --> PROVIDERS
    PLAY --> SPEAKERS
```

## Technology Stack

| Layer | Technology | Purpose |
|-------|-----------|---------|
| UI | QML + Kirigami | Declarative, Plasma-native interface |
| Backend | C++20 | WebSocket client, data models, controllers |
| Build | CMake + ECM | KDE-standard build system |
| Packaging | RPM | Fedora distribution |
| Communication | WebSocket JSON-RPC | Real-time bidirectional with MA server |
| Images | Qt Image Provider | Async album art loading via HTTP |

## Component Overview

```
src/
├── main.cpp                 # App bootstrap, singleton wiring
├── maclient.h/cpp           # WebSocket client (core)
├── playercontroller.h/cpp   # Player state & commands
├── queuecontroller.h/cpp    # Queue state & commands
├── librarycontroller.h/cpp  # Library browsing & search
├── mediaitemmodel.h/cpp     # QAbstractListModel for media items
├── playermodel.h/cpp        # QAbstractListModel for players
├── queueitemmodel.h/cpp     # QAbstractListModel for queue items
├── imageprovider.h/cpp      # QQuickAsyncImageProvider
└── qml/
    ├── Main.qml             # App window, navigation, bottom bar
    ├── NowPlayingPage.qml   # Current track, art, controls
    ├── LibraryPage.qml      # Tabbed library browser
    ├── MediaItemDelegate.qml # Reusable list item delegate
    ├── QueuePage.qml        # Queue management
    ├── PlayersPage.qml      # Player list & volume
    └── SettingsPage.qml     # Server connection config
```

## Data Flow

### Connection Lifecycle

```mermaid
sequenceDiagram
    participant App as Music Assistant Native
    participant MA as Music Assistant Server

    App->>MA: WebSocket connect to /ws
    MA->>App: ServerInfoMessage (version, name)
    App->>MA: auth {token: "..."}
    MA->>App: {authenticated: true, user: {...}}
    MA->>App: Events stream (player_updated, queue_updated, ...)

    loop User Interaction
        App->>MA: Command (e.g. players/all)
        MA->>App: Result (JSON)
    end
```

### Real-Time Updates

After authentication, the server pushes events automatically:

- **`player_updated`** — player state changes (volume, playback state, current track)
- **`queue_updated`** — queue state changes (shuffle, repeat, current index)
- **`queue_items_updated`** — queue contents changed (triggers re-fetch)
- **`media_item_added/updated/deleted`** — library changes

The controllers listen for these events and update their properties, which triggers QML UI updates via Qt's property binding system.

### Image Loading

```mermaid
sequenceDiagram
    participant QML as QML Image
    participant IP as MaImageProvider
    participant MA as MA Server /imageproxy

    QML->>IP: Request "image://ma/path|provider"
    IP->>MA: GET /imageproxy?path=...&provider=...&size=300
    Note over IP,MA: Authorization: Bearer <token>
    MA->>IP: JPEG/PNG image data
    IP->>QML: QImage → texture
```
