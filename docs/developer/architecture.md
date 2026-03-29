# Architecture

## Overview

Music Assistant Native is a native Qt6/KDE Frameworks 6 application that acts as both a **local audio player** and a **remote control** for a Music Assistant server. The app registers as a real MA player via the Sendspin protocol and can play audio through the PC speakers.

```mermaid
graph TB
    subgraph "Music Assistant Native (Desktop App)"
        QML["QML / Kirigami UI"]
        CPP["C++ Backend"]
        SS["SendspinClient"]
        AD["AudioDecoder"]
        MP["QMediaPlayer"]
    end

    subgraph "Music Assistant Server"
        WS["WebSocket API (/ws)"]
        IMGP["Image Proxy (/imageproxy)"]
        LIB["Music Library"]
        PLAY["Player Manager"]
        SSP["Sendspin Protocol (port 8927)"]
    end

    subgraph "External"
        SPEAKERS["Speakers (Sonos, AirPlay, etc.)"]
        PROVIDERS["Music Providers (Spotify, Tidal, etc.)"]
        PCAUDIO["PC Speakers"]
    end

    QML --> CPP
    CPP -->|"JSON-RPC over WS"| WS
    QML -->|"MaClient.getImageUrl()"| IMGP
    WS --> LIB
    WS --> PLAY
    LIB --> PROVIDERS
    PLAY --> SPEAKERS
    SSP -->|"FLAC binary frames"| SS
    SS --> AD
    AD --> MP
    MP --> PCAUDIO
```

## Technology Stack

| Layer | Technology | Purpose |
|-------|-----------|---------|
| UI | QML + Kirigami | Declarative, Plasma-native interface |
| Backend | C++20 | WebSocket client, data models, controllers |
| Build | CMake + ECM | KDE-standard build system |
| Audio | Qt6 Multimedia | Local FLAC playback via QMediaPlayer |
| Packaging | RPM, DEB, AppImage | Linux distribution |
| Communication | WebSocket JSON-RPC | Real-time bidirectional with MA server |
| Audio Protocol | Sendspin (WebSocket) | Registers as MA player, receives FLAC audio frames |
| Images | QML Image + getImageUrl() | Direct image loading via MA image proxy URL |

## Component Overview

```
src/
├── main.cpp                 # App bootstrap, singleton wiring
├── maclient.h/cpp           # WebSocket client (core)
├── sendspinclient.h/cpp     # Sendspin audio protocol client
├── audiodecoder.h/cpp       # FLAC audio decoding + QMediaPlayer playback
├── playercontroller.h/cpp   # Player state & commands
├── queuecontroller.h/cpp    # Queue state & commands
├── librarycontroller.h/cpp  # Library browsing & search
├── mediaitemmodel.h/cpp     # QAbstractListModel for media items
├── playermodel.h/cpp        # QAbstractListModel for players
├── queueitemmodel.h/cpp     # QAbstractListModel for queue items
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

Images are loaded directly via QML `Image` elements. The `MaClient.getImageUrl()` method builds the full image proxy URL (including authentication), and QML loads it as a standard HTTP image source — no custom image provider needed.

### Sendspin Audio Pipeline

```mermaid
sequenceDiagram
    participant MA as MA Server (port 8927)
    participant SS as SendspinClient
    participant AD as AudioDecoder
    participant MP as QMediaPlayer
    participant SPK as PC Speakers

    SS->>MA: WebSocket connect (Sendspin protocol)
    MA->>SS: Player registered in MA player list
    MA->>SS: Binary FLAC audio frames
    SS->>AD: Write frames to temp file
    AD->>MP: Set temp file as media source
    MP->>SPK: Audio output
```
