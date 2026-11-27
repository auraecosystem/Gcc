#!/bin/sh
set -eu

if [ -z "${GCC_SOURCE_DIR:-}" ]; then
    echo "error: set GCC_SOURCE_DIR to an existing local GCC source tree" >&2
    echo "No source repository will be cloned or fetched by this script." >&2
    exit 2
fi

SOURCE_DIR=$(CDPATH= cd -- "$GCC_SOURCE_DIR" && pwd)
if [ ! -x "$SOURCE_DIR/configure" ]; then
    echo "error: '$SOURCE_DIR/configure' is missing or not executable" >&2
    exit 2
fi

BUILD_DIR=${GCC_BUILD_DIR:-"$PWD/build-gcc"}
PREFIX=${GCC_PREFIX:-"$HOME/gcc-install"}
CONFIG=${BUILD_CONFIG:-bootstrap-Og}
JOBS=${JOBS:-2}

mkdir -p "$BUILD_DIR"
BUILD_DIR=$(CDPATH= cd -- "$BUILD_DIR" && pwd)
cd "$BUILD_DIR"

"$SOURCE_DIR/configure" \
    --prefix="$PREFIX" \
    --enable-languages=${GCC_LANGUAGES:-c,c++} \
    --disable-multilib \
    --with-build-config="$CONFIG"

make -j"$JOBS"
make check
make install

echo "GCC build, test, and installation steps completed."
