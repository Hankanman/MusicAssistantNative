#!/bin/bash
pkill -f musicassistant-native 2>/dev/null
sleep 1
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
echo "=== Starting app with debug logging ==="
QT_LOGGING_RULES="*.debug=true;qt.qpa.*=false;qt.multimedia.*=false" \
    ./build/bin/musicassistant-native 2>&1
