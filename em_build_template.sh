#!/bin/bash
#compile to /build folder
#
#Use this build command when using external resources (like images, wavs, fonts) that are stored in ./resources
#Build with custom stack size
#/usr/emsdk/upstream/emscripten/emcc rftest.c -I/usr/emsdk/upstream/emscripten/system/local/include -L/usr/emsdk/upstream/emscripten/system/local/lib --preload-file resources -o build/rftest.html --shell-file build/pgshell.html --use-port=sdl2 --use-port=sdl2_ttf --use-port=sdl2_gfx -s STACK_SIZE=262144
#
#Build with standard stack size
#/usr/emsdk/upstream/emscripten/emcc -g rftest.c -I/usr/emsdk/upstream/emscripten/system/local/include -L/usr/emsdk/upstream/emscripten/system/local/lib --preload-file resources -o build/rftest.html --shell-file build/pgshell.html --use-port=sdl2 --use-port=sdl2_ttf --use-port=sdl2_gfx

#Build with DWARF debug data and path to local files
/usr/emsdk/upstream/emscripten/emcc -g rftest.c -I/usr/emsdk/upstream/emscripten/system/local/include -L/usr/emsdk/upstream/emscripten/system/local/lib --preload-file resources -o build/rftest.html --shell-file build/pgshell.html --use-port=sdl2 --use-port=sdl2_ttf --use-port=sdl2_gfx
#
#
#If you are serving the webpage from the same machine you are building on, modify and use the two below statements to automatically copy the /build contents to your static html folder.
#
#delete previous build from html server
#sudo rm /var/www/[PATH TO YOUR PUBLIC STATIC HTML FOLDER]/rftest.*
#
#copy files to static html folder
#cp build/rftest.* /var/www/[PATH TO YOUR PUBLIC STATIC HTML FOLDER]
