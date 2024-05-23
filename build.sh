#!/bin/bash

set -xe

CC=g++
CFLAGS="-Wall -Wextra -Werror -pedantic -std=c++2b -ggdb"
INCLUDE="-I./raylib/src"
FRAMEWORKS="-framework OpenGL -framework GLUT -framework Cocoa -framework CoreVideo -framework IOKit"
MAIN_FILE="main.cc"
${CC} ${CFLAGS} ${FRAMEWORKS} ${INCLUDE} ./raylib/src/libraylib.a ${MAIN_FILE} -o ${MAIN_FILE/.cc/}