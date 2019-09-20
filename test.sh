#!/bin/bash

set -e

make clean
make debug

echo "*** DUMPING MEMORY ***"
hexdump -C memory.dump
