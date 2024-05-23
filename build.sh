#!/bin/bash

export PATH=$(pwd)/tools/armv5-eabi--glibc--stable-2020.08-1/bin:$PATH

make clean
make -j
