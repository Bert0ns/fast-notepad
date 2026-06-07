#!/bin/sh

mkdir -p build-tests
cd build-tests || exit

cmake .. -DBUILD_TESTING=ON -DENABLE_COVERAGE=ON
cmake --build . --target fast_notepad_tests
ctest --output-on-failure

echo "Generating coverage report..."
mkdir -p coverage
gcovr -r .. --object-directory . \
                --filter ".*/src/.*" \
                --xml coverage/coverage.xml \
                --html-details coverage/index.html \
                --print-summary
