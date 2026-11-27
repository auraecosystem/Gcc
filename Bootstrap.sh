#!/bin/sh
set -e

mkdir -p build-aux

aclocal
autoconf
automake --add-missing --copy

autom4te --language=autotest \
    -I tests \
    -o testsuite \
    tests/testsuite.at

chmod +x testsuite

echo "Bootstrap completed."
