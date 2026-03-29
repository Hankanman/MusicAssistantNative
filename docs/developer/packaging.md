# Packaging

Music Assistant Native is packaged for three distribution formats to cover all Linux users.

## Overview

| Format | Target | File |
|--------|--------|------|
| **RPM** | Fedora, openSUSE, RHEL | `musicassistant-native.spec` |
| **DEB** | Ubuntu, Debian, Mint | `debian/` directory |
| **AppImage** | Any Linux | `scripts/build-appimage.sh` |

## RPM (Fedora)

### Build

```bash
mkdir -p ~/rpmbuild/{SOURCES,SPECS}
cd /path/to/parent
tar --transform 's,^MusicAssistantNative,musicassistant-native-0.1.0,' \
    --exclude='build' --exclude='.git' --exclude='*.rpm' \
    -czf ~/rpmbuild/SOURCES/musicassistant-native-0.1.0.tar.gz MusicAssistantNative/
cp musicassistant-native.spec ~/rpmbuild/SPECS/
rpmbuild -bb ~/rpmbuild/SPECS/musicassistant-native.spec
```

### Install

```bash
sudo dnf install ~/rpmbuild/RPMS/x86_64/musicassistant-native-0.1.0-1.fc43.x86_64.rpm
```

### Spec File (`musicassistant-native.spec`)

| Section | Contents |
|---------|----------|
| `BuildRequires` | All Qt6 and KF6 `-devel` packages (including `qt6-qtmultimedia-devel`), CMake, ECM |
| `Requires` | kirigami, qtwebsockets, qtdeclarative, qt6-qtmultimedia |
| `%build` | `%cmake` + `%cmake_build` |
| `%check` | `desktop-file-validate` |

### Clean Build with Mock

```bash
mock -r fedora-43-x86_64 --buildsrpm \
    --spec musicassistant-native.spec \
    --sources ~/rpmbuild/SOURCES/
mock -r fedora-43-x86_64 --rebuild ~/rpmbuild/SRPMS/musicassistant-native-*.src.rpm
```

## DEB (Ubuntu/Debian)

### Build

```bash
# Install build dependencies
sudo apt-get install cmake extra-cmake-modules g++ \
    qt6-base-dev qt6-declarative-dev qt6-websockets-dev qt6-multimedia-dev \
    libkf6kirigami-dev libkf6i18n-dev libkf6coreaddons-dev \
    libkf6config-dev libkf6dbusaddons-dev libkf6notifications-dev \
    libkf6windowsystem-dev libkf6iconthemes-dev \
    dpkg-dev debhelper fakeroot

# Build the package
dpkg-buildpackage -us -uc -b
```

### Install

```bash
sudo dpkg -i ../musicassistant-native_0.1.0-1_amd64.deb
sudo apt-get -f install  # resolve any missing dependencies
```

### Debian Directory Structure

```
debian/
├── control          # Package metadata, build/runtime deps
├── rules            # Build script (uses dh + cmake)
├── changelog        # Version history
├── copyright        # License info
├── compat           # Debhelper compat level (13)
└── source/
    └── format       # Source format (3.0 native)
```

## AppImage (Universal Linux)

### Build

```bash
./scripts/build-appimage.sh
```

The script:

1. Downloads `linuxdeploy` and the Qt plugin automatically
2. Builds the project with CMake
3. Installs to an `AppDir` structure
4. Bundles all Qt6/KF6 libraries
5. Produces a self-contained `.AppImage` file

### Run

```bash
chmod +x MusicAssistantNative-0.1.0-x86_64.AppImage
./MusicAssistantNative-0.1.0-x86_64.AppImage
```

No installation required — works on any Linux with FUSE support.

### Manual AppImage Build

```bash
# Build and install to AppDir
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j$(nproc)
DESTDIR=AppDir cmake --install build

# Download tools
wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget -q https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy*.AppImage

# Create AppImage
QMAKE=qmake6 VERSION=0.1.0 ./linuxdeploy-x86_64.AppImage \
    --appdir AppDir \
    --desktop-file AppDir/usr/share/applications/io.github.musicassistant.native.desktop \
    --icon-file AppDir/usr/share/icons/hicolor/scalable/apps/musicassistant-native.svg \
    --plugin qt \
    --output appimage
```

## Installed Files (all formats)

```
/usr/bin/musicassistant-native
/usr/share/applications/io.github.musicassistant.native.desktop
/usr/share/metainfo/io.github.musicassistant.native.metainfo.xml
/usr/share/icons/hicolor/scalable/apps/musicassistant-native.svg
```

## Versioning

To bump the version, update these locations:

1. `project(musicassistant-native VERSION x.y.z)` in `CMakeLists.txt`
2. `Version:` in `musicassistant-native.spec`
3. `KAboutData` version in `src/main.cpp`
4. `<release>` entry in `io.github.musicassistant.native.metainfo.xml`
5. `debian/changelog` version
6. `VERSION` in `scripts/build-appimage.sh` and `AppImageBuilder.yml`

## CI/CD

All three formats are built automatically in GitHub Actions:

| Job | Container | Output |
|-----|-----------|--------|
| `build-rpm` | `fedora:43` | `.rpm` artifact |
| `build-deb` | `ubuntu:24.10` | `.deb` artifact |
| `build-appimage` | `ubuntu-24.04` runner | `.AppImage` artifact |

Artifacts are downloadable from the Actions tab on every push to `main` or PR.
