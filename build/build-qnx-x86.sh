#!/bin/bash

if [ "x${QNX_TARGET}" == "x" ]; then
  if [ ! -f "${HOME}/qnx660/qnx660-env.sh" ]; then
    echo "Can not find ${HOME}/qnx660/qnx660-env.sh"
    exit 1
  fi
  source ~/qnx660/qnx660-env.sh
fi

./configure --crossbuild=QNX:6.5.0:x86 --with-cc="i486-pc-nto-qnx6.5.0-gcc" --with-ld-opt="-lsocket" --without-http_rewrite_module

make -j8
