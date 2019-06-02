#!/bin/bash

echo "Compiling 32bit shared library\n"
gcc -fPIC -shared -nostartfiles -m32 -O3 monitor.c -o monitor_x86.so -ldl
strip -s monitor_x86.so
echo "Compiling 64bit shared library\n"
gcc -fPIC -shared -nostartfiles -m64 -O3 monitor.c -o monitor_x64.so -ldl
strip -s monitor_x64.so
echo "Build complete!\n"

