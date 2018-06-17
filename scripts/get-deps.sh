#!/usr/bin/bash
set -e

MPFR_VERSION="4.0.1"
GMP_VERSION="6.1.2"
MPC_VERSION="1.1.0"

DEP_DIR="$PWD/deps"
ROOT_DIR="$DEP_DIR/root"

CURL=`which curl`
TAR=`which tar`
MAKE=`which make`

mkdir -p "$DEP_DIR"

if [[ ! -d $DEP_DIR/mpfr-$MPFR_VERSION ]]; then
    $CURL "https://www.mpfr.org/mpfr-current/mpfr-$MPFR_VERSION.tar.xz" | $TAR -xJC $DEP_DIR
fi

if [[ ! -d $DEP_DIR/gmp-$GMP_VERSION ]]; then
    $CURL "https://gmplib.org/download/gmp/gmp-$GMP_VERSION.tar.xz" | $TAR -xJC $DEP_DIR
fi

if [[ ! -d $DEP_DIR/mpc-$MPC_VERSION ]]; then
    $CURL "https://ftp.gnu.org/gnu/mpc/mpc-$MPC_VERSION.tar.gz" | $TAR -xzC $DEP_DIR
fi

COMMON_CONFIG="--disable-shared --enable-static --prefix=$ROOT_DIR"

# Build GMP
if [[ ! -f $ROOT_DIR/lib/libgmp.a ]]; then
    pushd $DEP_DIR/gmp-$GMP_VERSION

    mkdir -p build && cd build
    ../configure $COMMON_CONFIG
    $MAKE
    $MAKE install
    popd
fi

# Build MPFR
if [[ ! -f $ROOT_DIR/lib/libmpfr.a ]]; then
    pushd $DEP_DIR/mpfr-$MPFR_VERSION

    mkdir -p build && cd build
    ../configure $COMMON_CONFIG --with-gmp=$ROOT_DIR
    $MAKE
    $MAKE check
    $MAKE install

    popd
fi

# Build MPC
if [[ ! -f $ROOT_DIR/lib/libmpc.a ]]; then
    pushd $DEP_DIR/mpc-$MPC_VERSION

    mkdir -p build && cd build
    ../configure $COMMON_CONFIG --with-mpfr=$ROOT_DIR --with-gmp=$ROOT_DIR
    $MAKE
    $MAKE check
    $MAKE install

    popd
fi
