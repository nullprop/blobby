#!/bin/bash

PLATFORM=${1:-"linux"}
ARGS=${2:-""}

# compile shaders

mkdir -p shader/build

# simple shader
./thirdparty/build/bin/shaderc \
-f shader/v_simple.sc -o shader/build/v_simple.bin \
--platform "$PLATFORM" --type vertex "$ARGS" -i ./

./thirdparty/build/bin/shaderc \
-f shader/f_simple.sc -o shader/build/f_simple.bin \
--platform "$PLATFORM" --type fragment "$ARGS" -i ./
