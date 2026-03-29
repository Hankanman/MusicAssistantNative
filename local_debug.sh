pkill -f musicassistant-native 
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
./build/bin/musicassistant-native