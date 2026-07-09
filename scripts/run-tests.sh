#!/bin/sh

mkdir -p build-tests
cd build-tests || exit

cmake .. -DBUILD_TESTING=ON -DENABLE_COVERAGE=ON
cmake --build . --parallel --target fast_notepad_tests
ctest --output-on-failure

echo "Generating coverage report..."
if command -v gcovr >/dev/null 2>&1; then
    rm -rf coverage
    mkdir -p coverage
    gcovr -r .. --object-directory . \
                    --filter ".*/src/.*" \
                    --xml coverage/coverage.xml \
                    --html-details coverage/index.html \
                    --gcov-ignore-errors=no_working_dir_found \
                    --print-summary
else
    echo "gcovr not found. Skipping coverage report."
fi
