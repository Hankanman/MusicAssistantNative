# Contributing

## Development Setup

1. Clone the repository
2. Install [build dependencies](building.md)
3. Build in debug mode:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Debug
   cmake --build build -j$(nproc)
   ```
4. Run:
   ```bash
   ./build/bin/musicassistant-native
   ```

## Project Structure

```
MusicAssistantNative/
├── CMakeLists.txt                          # Top-level CMake
├── src/
│   ├── CMakeLists.txt                      # Source build rules
│   ├── main.cpp                            # Entry point, wiring
│   ├── maclient.h/cpp                      # WebSocket client
│   ├── sendspinclient.h/cpp                # Sendspin audio protocol client
│   ├── audiodecoder.h/cpp                  # FLAC decoding + QMediaPlayer playback
│   ├── *controller.h/cpp                   # Business logic
│   ├── *model.h/cpp                        # Data models for QML
│   └── qml/*.qml                           # UI pages
├── tests/                                  # Integration test suite
├── docs/                                   # MkDocs documentation
├── .github/workflows/ci.yml               # CI pipeline
├── io.github.musicassistant.native.desktop       # Desktop entry
├── io.github.musicassistant.native.metainfo.xml  # AppStream metadata
├── musicassistant-native.spec                  # RPM spec
└── mkdocs.yml                              # Documentation config
```

## Code Style

- **C++20** with Qt conventions
- Use `QStringLiteral()` for string literals
- Use `Q_EMIT` instead of `emit`
- Use `Q_PROPERTY` for QML-exposed properties
- All user-visible strings wrapped in `i18n()`
- Follow the [KDE HIG](https://develop.kde.org/hig/) for UI patterns

## Adding a Feature

### New API Command

1. Add the method to the appropriate controller (`playercontroller`, `queuecontroller`, or `librarycontroller`)
2. Use `m_client->sendCommand()` with a callback
3. Expose the method to QML with `Q_INVOKABLE`
4. Add a test in `tests/`

### New UI Page

1. Create `src/qml/NewPage.qml`
2. Add it to `qt_add_qml_module` in `src/CMakeLists.txt`
3. Add a `Component` and navigation action in `Main.qml`
4. If it needs new data, create/extend a controller class

### New Data Model

1. Subclass `QAbstractListModel`
2. Define roles in an enum with `Q_ENUM`
3. Implement `rowCount()`, `data()`, `roleNames()`
4. Add it to `src/CMakeLists.txt`
5. Expose to QML via `main.cpp` context property

## Testing Changes

```bash
# Run the integration test suite
cd tests
MA_URL="http://your-server:8095" MA_TOKEN="your-token" python3 -m pytest -v

# Validate the desktop file
desktop-file-validate io.github.musicassistant.native.desktop

# Build the RPM to ensure packaging works
rpmbuild -bb musicassistant-native.spec
```

## Pull Request Checklist

- [ ] Code compiles without warnings (`-Wall -Werror=format-security`)
- [ ] New user-visible strings use `i18n()`
- [ ] New QML files added to `src/CMakeLists.txt`
- [ ] Desktop file validates
- [ ] RPM builds successfully
- [ ] Integration tests pass (if applicable)
- [ ] Documentation updated (if adding features)

## Future Work

Areas that would benefit from contributions:

- **MPRIS2 integration** — expose playback controls to Plasma's media widget
- **Album/artist detail pages** — drill-down views with track listings
- **Drag-and-drop queue reordering** — DnD in the queue list
- **Player grouping UI** — group/ungroup players from the app
- **Playlist management** — create, edit, delete playlists
- **OAuth browser flow** — in-app browser window for Home Assistant OAuth
- **Flatpak packaging** — for cross-distro distribution
