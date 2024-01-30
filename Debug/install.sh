#!/bin/bash

LIB_DIR=/usr/local/lib/iors_common
INC_DIR=/usr/local/include/iors_common

if [ ! -d $LIB_DIR ]; then
    mkdir $LIB_DIR
fi
if [ ! -d $INC_DIR ]; then
    mkdir $INC_DIR
fi

cp ../inc/*.h /usr/local/include/iors_common
cp libiors_common.so /usr/local/lib/iors_common
