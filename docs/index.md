# Music Assistant Native

A native KDE Plasma desktop client for [Music Assistant](https://music-assistant.io/) — the open-source music library manager from the Home Assistant ecosystem.

## What is this?

Music Assistant Native connects to your Music Assistant server and provides a Plasma-native interface for:

- **Browsing** your unified music library — artists, albums, tracks, playlists, and radios from all your connected sources (Spotify, Tidal, Qobuz, Plex, local files, and more)
- **Controlling playback** on any connected speaker — Sonos, AirPlay, Chromecast, DLNA, Snapcast, Alexa, and more
- **Managing your play queue** with shuffle, repeat, reordering, and search
- **Switching between players** with per-player volume control

!!! note "Remote control, not a local player"
    This app is a **remote control** for your Music Assistant server. Audio streams directly from the server to your speakers — the desktop app does not play audio locally.

## Quick Start

=== "RPM (Fedora)"

    ```bash
    sudo dnf install ./musicassistant-native-0.1.0-1.fc43.x86_64.rpm
    ```

=== "DEB (Ubuntu/Debian)"

    ```bash
    sudo dpkg -i musicassistant-native_0.1.0-1_amd64.deb
    sudo apt-get -f install
    ```

=== "AppImage (Any Linux)"

    ```bash
    chmod +x MusicAssistantNative-0.1.0-x86_64.AppImage
    ./MusicAssistantNative-0.1.0-x86_64.AppImage
    ```

=== "Build from source"

    ```bash
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build build -j$(nproc)
    sudo cmake --install build
    ```

Then launch `musicassistant-native`, enter your server URL and authentication token, and start listening.

## Requirements

| Component | Minimum Version |
|-----------|----------------|
| Qt | 6.6+ |
| KDE Frameworks | 6.0+ |
| Music Assistant Server | 2.x |
| Linux Desktop | KDE Plasma 6 (recommended) |

## Documentation

<div class="grid cards" markdown>

- :material-account-circle: **[User Guide](user/getting-started.md)**

    Installation, setup, and daily usage

- :material-code-braces: **[Developer Guide](developer/architecture.md)**

    Architecture, building, API reference, and contributing

</div>
