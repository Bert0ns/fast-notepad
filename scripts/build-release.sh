#!/bin/sh

mkdir -p build
cd build || exit
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --parallel
echo "run the app with ./build/FastNotepad"
