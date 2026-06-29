#!/bin/sh

mkdir -p build
cd build
cmake ..
cmake --build . --config Release
echo "run the app with ./build/FastNotepad"
