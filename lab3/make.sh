#!/usr/bin/sh
gcc matrix_gen.c -o mgen
gcc mmult-single.c -o single 
gcc matrix_mult.c -o mmult
