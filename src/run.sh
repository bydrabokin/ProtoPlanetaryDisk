#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 filename.c"
    exit 1
fi

gcc "$1" -o main $(sdl2-config --cflags --libs) \
    -lSDL2_ttf \
    -lSDL2_mixer \
    -lSDL2_image \
    -lm \
&& ./main
