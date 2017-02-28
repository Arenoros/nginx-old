#!/bin/bash

if [ ! -d "/usr/arm-linux-gnueabihf" ]; then
  echo "Can not find /usr/arm-linux-gnueabihf"
  exit 1
fi

#zlib
export CC=arm-linux-gnueabihf-gcc
./configure --prefix=/home/dan/nginx/lib/linux/armv7l/zlib
make install
#pcre
./configure --host=arm-linux-gnueabihf --prefix=/home/dan/nginx-cross/lib/linux/armv7l/pcre --disable-cpp
make install
#openssl
export cross=arm-linux-gnueabihf-
./Configure dist --prefix=/home/dan/nginx/lib/linux/armv7l/openssl
make CC="${cross}gcc" AR="${cross}ar r" RANLIB="${cross}ranlib"
make install

auto/configure --crossbuild=Linux::armv7l --with-cc="arm-linux-gnueabihf-gcc" --prefix=nginx --add-module=modules/nginx-let-module --with-http_v2_module --with-http_ssl_module

make -j8
