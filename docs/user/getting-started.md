# Getting Started

## Prerequisites

Before installing Music Assistant Native, you need:

1. **A running Music Assistant server** (v2.x) — either as a [Home Assistant add-on](https://music-assistant.io/integration/installation/) or [standalone](https://music-assistant.io/integration/installation/)
2. **A Linux desktop** with KDE Plasma 6 (other Qt-compatible desktops also work)
3. **Network access** to your Music Assistant server

## Installation

### From RPM (Fedora 42+)

```bash
sudo dnf install ./musicassistant-native-0.1.0-1.fc43.x86_64.rpm
```

### From DEB (Ubuntu 24.10+ / Debian)

```bash
sudo dpkg -i musicassistant-native_0.1.0-1_amd64.deb
sudo apt-get -f install
```

### AppImage (Any Linux)

```bash
chmod +x MusicAssistantNative-0.1.0-x86_64.AppImage
./MusicAssistantNative-0.1.0-x86_64.AppImage
```

### From Source

Install build dependencies:

```bash
sudo dnf install cmake extra-cmake-modules gcc-c++ \
    qt6-qtbase-devel qt6-qtdeclarative-devel \
    qt6-qtwebsockets-devel kf6-kirigami-devel \
    kf6-ki18n-devel kf6-kcoreaddons-devel kf6-kconfig-devel \
    kf6-kdbusaddons-devel kf6-knotifications-devel \
    kf6-kwindowsystem-devel kf6-kiconthemes-devel
```

Build and install:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j$(nproc)
sudo cmake --install build
```

Or run without installing:

```bash
./build/bin/musicassistant-native
```

## First Launch

1. Launch **Music Assistant Native** from your application menu or terminal
2. The app opens on the **Settings** page
3. Follow the [Connecting to Music Assistant](connecting.md) guide to set up your server connection
4. Once connected, use the sidebar to navigate between Now Playing, Library, Queue, and Players

## Runtime Dependencies

These are pulled in automatically by the RPM, but if building from source:

| Package | Purpose |
|---------|---------|
| `qt6-qtwebsockets` | WebSocket communication with MA server |
| `qt6-qtdeclarative` | QML UI engine |
| `kf6-kirigami` | KDE Kirigami UI components |
