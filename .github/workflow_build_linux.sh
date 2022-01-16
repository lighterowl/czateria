#!/usr/bin/env bash

set -e
set -x

for c in g++ clang++; do
  for t in Debug Release; do
    pushd "build_${c}_${t}"
    CXX=$c cmake -DCMAKE_BUILD_TYPE=$t -GNinja ..
    ninja
    popd
  done
done
