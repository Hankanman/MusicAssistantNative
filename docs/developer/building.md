# Building from Source

## Build Dependencies

### Fedora

```bash
sudo dnf install cmake extra-cmake-modules gcc-c++ \
    qt6-qtbase-devel qt6-qtdeclarative-devel \
    qt6-qtwebsockets-devel qt6-qtmultimedia-devel kf6-kirigami-devel \
    kf6-ki18n-devel kf6-kcoreaddons-devel kf6-kconfig-devel \
    kf6-kdbusaddons-devel kf6-knotifications-devel \
    kf6-kwindowsystem-devel kf6-kiconthemes-devel \
    desktop-file-utils
```

### Required Versions

| Dependency | Minimum Version |
|-----------|----------------|
| CMake | 3.20 |
| Qt 6 | 6.6.0 |
| KDE Frameworks 6 | 6.0.0 |
| Extra CMake Modules | 6.0.0 |
| GCC | C++20 capable (GCC 10+) |

## Build Commands

### Debug Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

### Release Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j$(nproc)
```

### Install

```bash
sudo cmake --install build
```

### Run Without Installing

```bash
# Set the Qt Quick style for proper KDE theming
QT_QUICK_CONTROLS_STYLE=org.kde.desktop ./build/bin/musicassistant-native
```

## CMake Structure

The project uses two `CMakeLists.txt` files:

### Top-level `CMakeLists.txt`

- Sets C++20, minimum Qt/KF versions
- Finds all Qt6 and KF6 packages via `find_package`
- Installs desktop file, metainfo, and icon
- Calls `add_subdirectory(src)`

### `src/CMakeLists.txt`

- Defines the executable with `add_executable`
- Registers QML module with `qt_add_qml_module`
- Links all Qt6 and KF6 libraries

## QML Module

The QML files are compiled into the binary as a Qt resource module (`io.github.musicassistant.native`). The QML is loaded at runtime via:

```cpp
engine.load(QUrl("qrc:/qt/qml/io/github/musicassistant/native/qml/Main.qml"));
```

## Troubleshooting Builds

### "Package not found" errors

Ensure you have the `-devel` packages installed. The CMake `find_package` calls look for `cmake(Qt6Core)`, etc., which map to the `-devel` RPM packages.

### QML module not loading

If the app compiles but shows a blank window, check:

1. The QML files are listed in `qt_add_qml_module` in `src/CMakeLists.txt`
2. The resource path matches what `engine.load()` expects
3. Run with `QML_IMPORT_TRACE=1` to see import attempts
