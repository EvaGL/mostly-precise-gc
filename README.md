DiplomaGC
=========

Non-conservative garbage collector for C++.

Installation:
=============

1. ./autogen.sh
2. ./configure
3. make clean | make | make install

Testing:
========

cd tests/boehmTest
make
LD_LIBRARY_PATH=/usr/local/lib ./output

