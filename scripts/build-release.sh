#!/bin/sh

mkdir -p build
cd build || exit
cmake ..
cmake --build . --config Release
echo "run the app with ./build/FastNotepad"
