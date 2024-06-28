#!/bin/bash
chmod +x clean.sh
rm -rf build
rm -rf command_line/build

if [ "$1" = "-f" ]; then
    rm CMakeLists.txt.user
    rm command_line/CMakeLists.txt.user
fi
