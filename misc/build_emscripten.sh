#!/bin/bash

# change me, if installed emscripten somewhere else
EM_DIR=~/src/emsdk
source "$EM_DIR/emsdk_env.sh"

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PRELOAD_DIR="$DIR/examples/data"
USE_FLAGS=(
    -s USE_FREETYPE=1
    -s USE_VORBIS=1
    -s USE_OGG=1
    -s USE_LIBJPEG=1
    -s USE_SDL=2
    -s USE_LIBPNG=1
    -s FULL_ES2=1
    -s ASYNCIFY
    -s TOTAL_MEMORY=2147418112
    -O3
    )


EM_CACHE="$EM_DIR/upstream/emscripten/cache"

emcmake cmake -S . -B build_emscripten \
    -D CMAKE_BUILD_TYPE=Release \
    -D ALLEGRO_SDL=ON \
    -D SHARED=OFF \
    -D WANT_MONOLITH=ON \
    -D WANT_ALLOW_SSE=OFF \
    -D WANT_DOCS=OFF \
    -D WANT_TESTS=OFF \
    -D WANT_OPENAL=OFF \
    -D ALLEGRO_WAIT_EVENT_SLEEP=ON \
    -D SDL2_INCLUDE_DIR=$EM_CACHE/sysroot/include \
    -D CMAKE_C_FLAGS="${USE_FLAGS[*]}" \
    -D CMAKE_CXX_FLAGS="${USE_FLAGS[*]}" \
    -D CMAKE_EXE_LINKER_FLAGS="${USE_FLAGS[*]} --preload-file $PRELOAD_DIR@/data" \
    -D CMAKE_EXECUTABLE_SUFFIX_CXX=".html"

echo finished configuring build
echo
echo run the following to build:
echo '  $ cmake --build build_emscripten'
echo then, start a web server for the examples:
echo '  $ cd build_emscripten/examples/Release; python3 -m http.server'
echo or the demos:
echo '  $ cd build_emscripten/demos; python3 -m http.server'
