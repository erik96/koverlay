#!/usr/bin/env bash

mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCPACK_GENERATOR=RPM ..
cmake --build . -j
cpack -G RPM
