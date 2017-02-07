#!/bin/bash
gcc -std=c99 -ggdb -I '../include' -fpic -c unvd_hc.h unvd_hc.c
gcc -shared -o libunvd_hc.so unvd_hc.o -lhcnetsdk -lhpr -lHCCore
gcc -std=c99 -ggdb -I '../include' -I './' -L'./' unvd_hc.h ../test.c -lunvd_hc
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:'./'
