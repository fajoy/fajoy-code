#!/bin/sh
mkdir -p ras/bin
g++ noop.cpp -o ras/bin/noop
g++ number.cpp -o ras/bin/number
g++ removetag.cpp -o ras/bin/removetag
g++ removetag0.cpp -o ras/bin/removetag0
cp test.html ras
cp /bin/cat ras/bin
cp /bin/ls ras/bin
