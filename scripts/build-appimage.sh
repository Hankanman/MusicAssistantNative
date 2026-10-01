#!/bin/bash
# Build an AppImage for Music Assistant Native
# Requires: linuxdeploy, linuxdeploy-plugin-qt
#
# Usage: ./scripts/build-appimage.sh
#
# Downloads tools automatically if not present.

set -euo pipefail

APPNAME="MusicAssistantNative"
VERSION="2026.04.08"
SRCDIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILDDIR="${SRCDIR}/build-appimage"
APPDIR="${BUILDDIR}/AppDir"

echo "=== Building ${APPNAME} AppImage v${VERSION} ==="

# Download linuxdeploy if needed
TOOLS_DIR="${BUILDDIR}/tools"
mkdir -p "${TOOLS_DIR}"

if [ ! -x "${TOOLS_DIR}/linuxdeploy" ]; then
    echo "Downloading linuxdeploy..."
    wget -q -O "${TOOLS_DIR}/linuxdeploy" \
        "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
    chmod +x "${TOOLS_DIR}/linuxdeploy"
fi

if [ ! -x "${TOOLS_DIR}/linuxdeploy-plugin-qt" ]; then
    echo "Downloading linuxdeploy-plugin-qt..."
    wget -q -O "${TOOLS_DIR}/linuxdeploy-plugin-qt" \
        "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
    chmod +x "${TOOLS_DIR}/linuxdeploy-plugin-qt"
fi

# Build the project
echo "=== Configuring ==="
cmake -B "${BUILDDIR}/cmake" -S "${SRCDIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr

echo "=== Building ==="
cmake --build "${BUILDDIR}/cmake" -j"$(nproc)"

echo "=== Installing to AppDir ==="
rm -rf "${APPDIR}"
DESTDIR="${APPDIR}" cmake --install "${BUILDDIR}/cmake"

# Create AppImage
echo "=== Creating AppImage ==="
export QMAKE=qmake6
export QML_SOURCES_PATHS="${SRCDIR}/src/qml"
export APPIMAGE_EXTRACT_AND_RUN=1
export VERSION

# Ensure Kirigami desktop style and addons modules are bundled into AppDir
mkdir -p "${APPDIR}/usr/qml/org/kde"
for qml_path in /usr/lib/x86_64-linux-gnu/qt6/qml /usr/lib64/qt6/qml /usr/lib/qt6/qml; do
    if [ -d "${qml_path}/org/kde/desktop" ]; then
        cp -r "${qml_path}/org/kde/desktop" "${APPDIR}/usr/qml/org/kde/" 2>/dev/null || true
    fi
    if [ -d "${qml_path}/org/kde/kirigamiaddons" ]; then
        cp -r "${qml_path}/org/kde/kirigamiaddons" "${APPDIR}/usr/qml/org/kde/" 2>/dev/null || true
    fi
done

cd "${BUILDDIR}"
"${TOOLS_DIR}/linuxdeploy" \
    --appdir "${APPDIR}" \
    --desktop-file "${APPDIR}/usr/share/applications/io.github.musicassistant.native.desktop" \
    --icon-file "${APPDIR}/usr/share/icons/hicolor/scalable/apps/musicassistant-native.svg" \
    --plugin qt \
    --output appimage

echo "=== Done ==="
ls -lh "${BUILDDIR}"/*.AppImage
