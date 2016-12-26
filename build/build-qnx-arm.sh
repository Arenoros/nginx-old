#!/bin/bash

if [ "x${QNX_TARGET}" == "x" ]; then
  if [ ! -f "${HOME}/qnx660/qnx660-env.sh" ]; then
    echo "Can not find ${HOME}/qnx660/qnx660-env.sh"
    exit 1
  fi
  source ~/qnx660/qnx660-env.sh
fi

auto/configure --crossbuild=QNX:6.5.0:arm --with-cc="arm-unknown-nto-qnx6.5.0-gcc" --with-ld-opt="-lsocket" --prefix=nginx --add-module=nginx-let-module

make -j8
