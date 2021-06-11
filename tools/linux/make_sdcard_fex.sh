#!/bin/sh
DIR_NAME=$( cd "$(dirname "$0")"  ; pwd)
dd if=$1 of=$2 skip=1 bs=220
cat $DIR_NAME/boot0_sdcard_head.bin $2 > make_$2 
