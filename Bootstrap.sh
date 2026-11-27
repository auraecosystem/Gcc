#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$ROOT"

for tool in autoreconf autom4te; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "error: required tool '$tool' was not found in PATH" >&2
        exit 1
    fi
done

mkdir -p build-aux
autoreconf --install --force

autom4te \
    --language=autotest \
    -I . \
    -I test \
    -o testsuite \
    test/Testsuite.at

chmod +x testsuite
echo "Bootstrap completed."
