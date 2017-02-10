#!/bin/bash
gcc -std=c11 -ggdb -I '../include' -fpic -c unvd_dh.h unvd_dh.c
gcc -shared -o libunvd_dh.so unvd_dh.o -ldhnetsdk
gcc -std=c11 -ggdb -I '../include' -I './' -L'./' unvd_dh.h ../tests/test_dh.c -lunvd_dh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:'./'
