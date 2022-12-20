#!/bin/sh
gcc src/master.c -lncurses -o bin/master
gcc src/command_console.c -lncurses -o bin/command
gcc src/inspection_console.c -lncurses -lm -o bin/inspection
gcc src/mx.c -o bin/mx
gcc src/mz.c -o bin/mz
gcc src/world.c -o bin/world


