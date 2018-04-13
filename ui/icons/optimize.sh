#!/usr/bin/env bash

set -e
set -x

trap deltmpdir EXIT
tmpdir=$(mktemp -d)
deltmpdir() {
  rm -fR "$tmpdir"
}

optipng -o7 ./*.png

pngcrush -brute -d "$tmpdir" ./*.png
mv -t . "$tmpdir"/*
