#!/bin/sh

mkdir tmp
cd tmp
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
mv compile_commands.json ../compile_commands.json
cd ..
rm -r tmp
