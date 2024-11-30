#!/bin/bash
set -e

cp tests/disk_images/b.img test.img

./ds3cp test.img tests/6kwords.txt 5
./ds3ls test.img /a/b
./ds3cat test.img 5
./ds3bits test.img
