#!/usr/bin/env bash

set -e
set -x

compiler=$1
qmakeflags=(CONFIG+=debug)

mkdir -p "build_${compiler}"
cd "build_${compiler}"
qmake ../czateria.pro -spec "linux-${compiler}" "${qmakeflags[@]}"
make -j$(grep -c ^processor /proc/cpuinfo)
