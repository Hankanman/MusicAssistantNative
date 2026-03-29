# Music Assistant Native

Native KDE Plasma desktop client for [Music Assistant](https://music-assistant.io/), an open-source music library manager in the Home Assistant ecosystem.

## Project Overview

- **Language**: C++20 + QML
- **UI Framework**: Qt 6 + KDE Frameworks 6 (Kirigami)
- **Build System**: CMake + ECM (Extra CMake Modules)
- **Packaging**: RPM (Fedora)
- **License**: GPL-3.0-or-later

## Architecture

```
┌─────────────────────────────────────────────┐
│              QML / Kirigami UI              │
│  (Main, NowPlaying, Library, Queue,        │
│   Players, Settings pages)                  │
├─────────────────────────────────────────────┤
│            C++ Backend Layer                │
│  ┌──────────┐ ┌──────────┐ ┌─────────────┐ │
│  │MaClient  │ │Models    │ │Sendspin     │ │
│  │(WebSocket│ │(Player,  │ │Client       │ │
│  │ JSON-RPC)│ │Queue,    │ │+ AudioDecoder│ │
│  │          │ │MediaItem)│ │(local audio)│ │
│  └──────────┘ └──────────┘ └─────────────┘ │
├─────────────────────────────────────────────┤
│       Music Assistant Server                │
│  WebSocket API (/ws) + Sendspin (:8927)     │
└─────────────────────────────────────────────┘
```

- **MaClient**: WebSocket JSON-RPC client for the MA control API (library, players, queues)
- **SendspinClient**: Registers as a Sendspin player, receives audio stream frames
- **AudioDecoder**: Decodes streamed audio (FLAC) via QMediaPlayer to local speakers

## Music Assistant API

- **Protocol**: WebSocket JSON-RPC at `ws://<host>:<port>/ws`
- **Auth**: JWT token (login with credentials or join code)
- **Key commands**: `music/*` (library), `players/*` (control), `player_queues/*` (queue)
- **Events**: Real-time push via WebSocket after auth (player_updated, queue_updated, etc.)

## Code Conventions

- Use `Q_OBJECT` macro for all QObject subclasses
- Expose C++ to QML via `QML_ELEMENT` / `QML_SINGLETON` macros
- All user-visible strings wrapped in `i18n()` / `i18nc()`
- Follow KDE HIG: use Kirigami components, Breeze icons, standard patterns
- JSON parsing with QJsonDocument (Qt built-in)
- WebSocket messages use `message_id` for request/response correlation

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j$(nproc)
./build/src/musicassistant-native
```

## RPM Build

```bash
rpmbuild -bb musicassistant-native.spec --define "_sourcedir $(pwd)"
```

## File Layout

```
CMakeLists.txt              # Top-level CMake
src/
  CMakeLists.txt            # Source build rules
  main.cpp                  # Entry point
  maclient.h/cpp            # WebSocket client for MA API
  playercontroller.h/cpp    # Player state & control
  queuecontroller.h/cpp     # Queue state & control
  librarycontroller.h/cpp   # Music library browsing
  mediaitemmodel.h/cpp      # QAbstractListModel for media items
  playermodel.h/cpp         # QAbstractListModel for players
  queueitemmodel.h/cpp      # QAbstractListModel for queue items
  sendspinclient.h/cpp      # Sendspin player registration + audio streaming
  audiodecoder.h/cpp        # Audio decoding via QMediaPlayer (FLAC/Opus)
  qml/
    Main.qml                # App window + navigation
    NowPlayingPage.qml      # Current track, controls, art
    LibraryPage.qml         # Browse artists/albums/tracks/playlists
    QueuePage.qml           # Current queue
    PlayersPage.qml         # Player selection & volume
    SettingsPage.qml        # Server connection settings
    MediaItemDelegate.qml   # Reusable delegate for media list items
io.github.musicassistant.native.desktop
io.github.musicassistant.native.metainfo.xml
musicassistant-native.spec      # RPM spec file
tests/                          # Python integration tests (pytest)
```
