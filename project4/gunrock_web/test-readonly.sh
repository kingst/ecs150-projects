#! /bin/bash
if ! LANG=C shasum --quiet -c tests/img.shas; then
    echo "Error: Modified disk image file found"
    exit 1
fi

if ! [[ -x ds3ls ]]; then
    echo "ds3ls executable does not exist"
    exit 1
fi

../tester/run-tests.sh $*


