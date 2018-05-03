#!/usr/bin/bash
set -e

MPFR_VERSION="4.0.1"
GMP_VERSION="6.1.2"

ROOT_DIR="$PWD/deps/root"

CURL=`which curl`
TAR=`which tar`
MAKE=`which make`

mkdir -p deps

if [[ ! -d deps/mpfr-$MPFR_VERSION ]]; then
    $CURL "http://www.mpfr.org/mpfr-current/mpfr-$MPFR_VERSION.tar.xz" | $TAR -xJC deps
fi

if [[ ! -d deps/gmp-$GMP_VERSION ]]; then
    $CURL "https://gmplib.org/download/gmp/gmp-$GMP_VERSION.tar.xz" | $TAR -xJC deps
fi

if [[ ! -f deps/cpplint.py ]]; then
    $CURL https://raw.githubusercontent.com/cpplint/cpplint/master/cpplint.py > deps/cpplint.py
fi

# Build GMP
if [[ ! -f $ROOT_DIR/usr/local/lib/libgmp.a ]]; then
    pushd deps/gmp-$GMP_VERSION

    mkdir -p build && cd build
    ../configure
    $MAKE
    $MAKE install DESTDIR="$ROOT_DIR"
    popd
fi

# Build MPFR
if [[ ! -f $ROOT_DIR/usr/local/lib/libmpfr.a ]]; then
    pushd deps/mpfr-$MPFR_VERSION

    mkdir -p build && cd build
    ../configure --with-gmp-build=../../gmp-$GMP_VERSION/build
    $MAKE
    $MAKE install DESTDIR="$ROOT_DIR"

    popd
fi