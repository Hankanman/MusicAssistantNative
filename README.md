# Music Assistant Native

A native KDE Plasma desktop client for [Music Assistant](https://music-assistant.io/) — the open-source music library manager from the Home Assistant ecosystem.

![License](https://img.shields.io/badge/license-GPL--3.0-blue)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20KDE%20Plasma-green)
![Qt](https://img.shields.io/badge/Qt-6.6%2B-41cd52)
![KDE Frameworks](https://img.shields.io/badge/KDE%20Frameworks-6.0%2B-1d99f3)

## Features

- Browse your unified music library — artists, albums, tracks, playlists, and radios
- Control playback on any Music Assistant player (Sonos, AirPlay, Chromecast, DLNA, Snapcast, etc.)
- Manage the play queue with shuffle, repeat, and reordering
- Now Playing view with album art, seek bar, and playback controls
- Player selection with per-player volume control and power toggle
- Search across all configured music providers
- Native KDE Plasma look and feel with Kirigami UI

## Screenshots

*Coming soon — the app opens to a Settings page where you connect to your server, then you can browse Library, Now Playing, Queue, and Players.*

## Requirements

- Fedora 42+ (or any Linux with Qt 6.6+ and KDE Frameworks 6)
- A running [Music Assistant](https://music-assistant.io/) server (v2.x)
- KDE Plasma desktop (recommended) or any Qt-compatible desktop

### Runtime dependencies

- `qt6-qtwebsockets`
- `qt6-qtdeclarative`
- `kf6-kirigami`

## Installation

### From RPM (Fedora)

```bash
sudo dnf install ./musicassistant-native-0.1.0-1.fc43.x86_64.rpm
```

### From source

```bash
# Install build dependencies
sudo dnf install cmake extra-cmake-modules gcc-c++ \
    qt6-qtbase-devel qt6-qtdeclarative-devel \
    qt6-qtwebsockets-devel kf6-kirigami-devel \
    kf6-ki18n-devel kf6-kcoreaddons-devel kf6-kconfig-devel \
    kf6-kdbusaddons-devel kf6-knotifications-devel \
    kf6-kwindowsystem-devel kf6-kiconthemes-devel

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j$(nproc)

# Install
sudo cmake --install build

# Or run directly without installing
./build/bin/musicassistant-native
```

## Connecting to Music Assistant

### Step 1: Find your Music Assistant server URL

Your Music Assistant server runs on a specific host and port. The default port is **8095**.

- **If running as a Home Assistant add-on**: The URL is typically `http://<your-ha-ip>:8095`
- **If running standalone**: The URL is `http://<server-ip>:8095`
- You can verify by opening the URL in a browser — you should see the Music Assistant web UI

### Step 2: Get authentication credentials

Music Assistant v2 uses JWT token authentication. There are three ways to authenticate:

#### Option A: Username and password (simplest)

If you have a Music Assistant user account with a password:

1. Open Music Assistant Native
2. In the **Settings** page, enter your server URL (e.g. `http://192.168.1.100:8095`)
3. Enter your **Username** and **Password**
4. Click **Connect**

The default admin account is created during Music Assistant's first-time setup. If you don't remember the credentials, see Option B or C.

#### Option B: Using a Join Code (recommended for new devices)

Join codes are 6-character pairing codes that make it easy to authorize new devices:

1. Open the **Music Assistant web UI** in your browser
2. Go to **Settings** > **Users & Authentication**
3. Click **Create Join Code**
4. Note the 6-character code (e.g. `ABC123`) — it expires after 8 hours

Then in Music Assistant Native:

1. Enter your server URL and click **Connect**
2. Use the join code to exchange for a token (this feature will be in a future release — for now, use the token approach below)

#### Option C: Using a long-lived token (most reliable)

This is the most reliable method and is recommended for desktop clients:

1. Open the **Music Assistant web UI** in your browser (`http://<server-ip>:8095`)
2. Click the **user icon** in the top-right corner
3. Go to **Settings** > **Users & Authentication** > **Tokens**
4. Click **Create Token**
5. Give it a name like "KDE Desktop Client"
6. **Copy the token** — it will only be shown once

Then in Music Assistant Native:

1. Enter your server URL (e.g. `http://192.168.1.100:8095`)
2. Paste the token into the **Token** field
3. Click **Connect**

You should see a green status message: "Connected to [Server Name] (v2.x.x)"

#### Option D: Get a token via the API directly

If you prefer the command line:

```bash
# Login and get a token
curl -X POST http://192.168.1.100:8095/api \
  -H "Content-Type: application/json" \
  -d '{
    "message_id": "1",
    "command": "auth/login",
    "args": {
      "username": "admin",
      "password": "your-password",
      "provider_id": "builtin",
      "device_name": "KDE Desktop"
    }
  }'

# The response contains a "token" field — copy it
```

Or create a long-lived token (after logging in with a short-lived one):

```bash
curl -X POST http://192.168.1.100:8095/api \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <your-short-lived-token>" \
  -d '{
    "message_id": "2",
    "command": "auth/token/create",
    "args": {"name": "KDE Desktop Client"}
  }'
```

### Step 3: Select a player

After connecting:

1. Navigate to the **Players** page via the sidebar
2. You'll see all available players from Music Assistant
3. Click a player to select it as your active player
4. The bottom playback bar will appear

### Step 4: Play music

1. Go to the **Library** page
2. Use the tabs to browse Artists, Albums, Tracks, Playlists, or Radios
3. Use the search bar to find specific music
4. Click any item to start playing it on your selected player

## Troubleshooting

### App won't start / no window appears

```bash
# Kill any existing instances first (only one instance allowed)
pkill -f musicassistant-native

# Run from terminal to see errors
musicassistant-native
```

### "Not connected" after entering credentials

- Verify the server URL is correct and includes the port: `http://192.168.1.100:8095`
- Make sure Music Assistant server is running
- Check if you can reach the server: `curl http://192.168.1.100:8095/info`
- If using a token, make sure it hasn't expired
- Check your firewall allows connections on port 8095

### No players showing up

- Make sure you have players configured in Music Assistant
- Check that players are powered on and available
- Click the Refresh button on the Players page
- Some players only appear when they're on the same network

### Library is empty

- Music Assistant needs to sync your music sources first
- Open the Music Assistant web UI and check if providers are configured
- Trigger a library sync from the web UI: Settings > Music Providers > Sync

## Architecture

```
┌─────────────────────────────────────────┐
│         QML / Kirigami UI               │
│  Now Playing, Library, Queue, Players,  │
│  Settings — all native Plasma widgets   │
├─────────────────────────────────────────┤
│           C++ Backend                   │
│  MaClient (WebSocket JSON-RPC)          │
│  PlayerController, QueueController,     │
│  LibraryController, Image Provider      │
├─────────────────────────────────────────┤
│     Music Assistant Server (v2)         │
│     WebSocket API at ws://host/ws       │
│     40+ music providers, 20+ player     │
│     types, unified library              │
└─────────────────────────────────────────┘
```

The app communicates with Music Assistant over WebSocket using JSON-RPC messages. All playback happens server-side — the app is a remote control, not a local audio player. Audio is streamed directly from the Music Assistant server to your speakers.

## Building an RPM

```bash
# Create source tarball
cd /path/to/parent && tar --transform 's,^MusicAssistantNative,musicassistant-native-0.1.0,' \
    --exclude='build' --exclude='*.rpm' \
    -czf ~/rpmbuild/SOURCES/musicassistant-native-0.1.0.tar.gz MusicAssistantNative/

# Build RPM
rpmbuild -bb musicassistant-native.spec
# Output: ~/rpmbuild/RPMS/x86_64/musicassistant-native-0.1.0-1.fc43.x86_64.rpm
```

## License

GPL-3.0-or-later

## Acknowledgments

- [Music Assistant](https://music-assistant.io/) — the server this client connects to
- [KDE Frameworks](https://develop.kde.org/) and [Kirigami](https://develop.kde.org/frameworks/kirigami/) — the UI toolkit
- [Qt](https://www.qt.io/) — the application framework
