#!/bin/bash

set -e

make clean
make debug -j

echo "*** DUMPING MEMORY ***"
hexdump -C memory.dump
