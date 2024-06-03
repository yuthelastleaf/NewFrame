#!/bin/sh
basepath=$(cd `dirname $0`;pwd)
echo ${basepath}

cd ${basepath}/x264   # 根据路径名称自行修改
pwd

./configure --prefix=${basepath}/x264_install --enable-shared
make -j8
make install