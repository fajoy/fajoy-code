#!/bin/sh
read -p "c code:" cfile
gcc "$cfile.c" -o "$cfile.o"
