#!/bin/bash
# Build an AppImage for Music Assistant Native
# Requires: linuxdeploy, linuxdeploy-plugin-qt
#
# Usage: ./scripts/build-appimage.sh
#
# Downloads tools automatically if not present.

set -euo pipefail

APPNAME="MusicAssistantNative"
VERSION="2026.03.29"
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
export VERSION

cd "${BUILDDIR}"
"${TOOLS_DIR}/linuxdeploy" \
    --appdir "${APPDIR}" \
    --desktop-file "${APPDIR}/usr/share/applications/io.github.musicassistant.native.desktop" \
    --icon-file "${APPDIR}/usr/share/icons/hicolor/scalable/apps/musicassistant-native.svg" \
    --plugin qt \
    --output appimage

echo "=== Done ==="
ls -lh "${BUILDDIR}"/*.AppImage
