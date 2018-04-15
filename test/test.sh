#!/usr/bin/bash

set -e
set -f

PRECISION=256

rebuild() {
    rm -rf build
    mkdir -p build
    pushd build
    cmake .. $1
    cmake --build .
    popd
}

print_banner() {
    echo
    echo
    echo "***************************************************************************"
    echo "$1"
    echo "***************************************************************************"
    echo
    echo
}

test_from_file() {
    cat $1 |
    while read line; do

        equation=`echo "$line" | cut -d= -f1`

        answer=`echo ${line#$equation'='} | cut -d' ' -f1`

        equation=`echo $equation | tr -d ''`
        if [ -z "$answer" ] || [ -z "$equation" ]; then
            continue
        fi
        printf "TEST: checking %s = %s..." "$equation" "$answer"
        status=`./build/simplify "$line"`
        if [[ "$status" -ne "1" ]]; then
            printf "FAILED\n"
            exit 1
        fi
        echo "SUCCESS"
    done
}

print_banner "INTEGER TEST, NO DEPENDENCIES"

rebuild -DDISABLE_GMP=ON -DDISABLE_MPFR=ON -DINTEGERS_ONLY=ON

test_from_file ./test/data/integers.txt
test_from_file ./test/data/ops_order.txt

print_banner "FLOATING POINT TEST, NO DEPENDENCIES"

rebuild -DDISABLE_GMP=ON -DDISABLE_MPFR=ON

test_from_file ./test/data/integers.txt
test_from_file ./test/data/small_floats.txt
test_from_file ./test/data/ops_order.txt


print_banner "INTEGER TEST, DEPENDS ON GMP"

rebuild -DDISABLE_MPFR=ON -DINTEGERS_ONLY=ON

test_from_file ./test/data/integers.txt
test_from_file ./test/data/ops_order.txt

print_banner "FLOATING POINT TEST, DEPENDS ON GMP, $PRECISION PRECISION BITS"

rebuild -DDISABLE_MPFR=ON -DFLOAT_PRECISION=$PRECISION


test_from_file ./test/data/integers.txt
test_from_file ./test/data/large_floats.txt
test_from_file ./test/data/ops_order.txt

print_banner "FLOATING POINT TEST, DEPENDS ON MPFR AND GMP, $PRECISION PRECISION BITS"

rebuild -DFLOAT_PRECISION=$PRECISION

test_from_file ./test/data/integers.txt
test_from_file ./test/data/yuge_floats.txt
test_from_file ./test/data/ops_order.txt