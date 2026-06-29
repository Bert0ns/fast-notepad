#!/bin/sh

mkdir -p build
cd build
cmake ..
cmake --build .
echo "run the app with ./build/FastNotepad"
