#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

mkdir -p "$ROOT_DIR/src/gen/embedded"

cd "$ROOT_DIR"

mkdir -p src/gen/embedded

xz --format=lzma -vfc6ekT 0 $ROOT_DIR/embedded/ramdisk.dmg > $ROOT_DIR/embedded/ramdisk-compressed.lzma
xz --format=lzma -vfc6ekT 0 $ROOT_DIR/embedded/checkra1n-kpf-pongo > $ROOT_DIR/embedded/checkra1n-kpf-pongo-compressed.lzma
jq -c . $ROOT_DIR/embedded/DFUHelperDeviceInfo.json > $ROOT_DIR/embedded/DFUHelperDeviceInfo-minified.json

for file in embedded/*; do
    name=$(basename "$file")
    symbol="embedded_${name//[^a-zA-Z0-9_]/_}"

    echo " XXD    $file"

    xxd -i "$file" \
        | sed -E \
            -e "s/^unsigned char .*\[\] =/static const unsigned char ${symbol}[] =/" \
            -e "s/^unsigned int .*_len =/static const size_t ${symbol}_len =/" \
        > "src/gen/embedded/${name%.*}.h"
done
