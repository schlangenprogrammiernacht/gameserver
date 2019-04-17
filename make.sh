#!/bin/sh

mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
#cmake -DCMAKE_BUILD_TYPE=Debug ..
make $@
