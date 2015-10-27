#!/bin/sh

g++ -Wno-write-strings -I./src $1 -o ./build/app -lGL `sdl2-config --cflags --libs`
./build/app
