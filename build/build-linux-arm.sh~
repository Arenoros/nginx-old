#!/bin/bash

if [ ! -d "/usr/arm-linux-gnueabihf" ]; then
  echo "Can not find /usr/arm-linux-gnueabihf"
  exit 1
fi

auto/configure --crossbuild=Linux::arm --with-cc="arm-linux-gnueabihf-gcc" --with-ld-opt="-L/usr/arm-linux-gnueabihf/lib" --with-cc-opt="-L/usr/arm-linux-gnueabihf/include" --with-pcre --add-module=modules/nginx-let-module

make -j8
