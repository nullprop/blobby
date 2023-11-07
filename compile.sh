#!/bin/sh

PLATFORM="${1:-"linux"}"
CONFIG="${2:-"debug"}"

./compile-shaders.sh "$PLATFORM"

./configure-ninja.sh
cmake --build "build/$CONFIG-ninja"
