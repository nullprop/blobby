#!/bin/bash

$CONFIG=${1:-"debug"}
$PLATFORM=${2:-"linux"}

./compile-shaders.sh "$PLATFORM"

./configure-ninja.sh
# cmake --build "build/$CONFIG-ninja"
cmake --build "build/debug-ninja"
