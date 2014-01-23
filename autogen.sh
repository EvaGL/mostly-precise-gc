#!/bin/bash

set -x
pushd `dirname $0`
libtoolize
aclocal
autoconf --force
automake --add-missing --copy --foreign
popd
