#!/bin/bash

BUILD_TYPE=${1:-release}

if [ "$BUILD_TYPE" = "release" ]; then
  FLAGS="-O3"
else
  FLAGS="-g -DDEBUG"
fi

mkdir -p build data docs

g++ -std=c++17 -Wall -Wextra -Wpedantic $FLAGS -D_XOPEN_SOURCE=700 -D_GNU_SOURCE -DPI=3.14159265358979323846 -Iinclude src/*.cpp -o build/speakerbox -lglog -lpthread

echo "Built speakerbox in build/ for $BUILD_TYPE"
