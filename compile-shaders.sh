#!/bin/bash

PLATFORM=${1:-"linux"}
ARGS=${2:-""}

# compile shaders

mkdir -p shaders/build

# simple shader
./thirdparty/build/bin/shaderc \
-f shaders/v_simple.sc -o shaders/build/v_simple.bin \
--platform "$PLATFORM" --type vertex "$ARGS" -i ./

./thirdparty/build/bin/shaderc \
-f shaders/f_simple.sc -o shaders/build/f_simple.bin \
--platform "$PLATFORM" --type fragment "$ARGS" -i ./
