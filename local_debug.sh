#!/bin/bash
pkill -f musicassistant-native 2>/dev/null
sleep 1

if [ ! -d build ] || [ "$1" = "--clean" ]; then
    rm -rf build
    cmake -B build -DCMAKE_BUILD_TYPE=Debug 2>&1 | tail -1
fi

cmake --build build -j$(nproc) 2>&1 | tail -1
echo "=== Starting Music Assistant Native ==="

QT_LOGGING_RULES="*.debug=true" \
    ./build/bin/musicassistant-native 2>&1 \
    | grep --line-buffered -E "MaClient:|SendspinClient:|QueueController:|PlayerController:|LibraryController:|LocalPlayer:|PlayerModel:|Play button|Item clicked|LibraryPage:|Auto-selected|qml:"
