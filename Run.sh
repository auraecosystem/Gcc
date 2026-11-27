#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$ROOT"

chmod +x ./Bootstrap.sh
./Bootstrap.sh
./configure
make
make check
