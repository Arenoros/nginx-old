#!/bin/bash

 auto/configure --crossbuild=QNX:6.5.0:arm --with-cc="arm-unknown-nto-qnx6.5.0-gcc" --with-ld-opt="-lsocket" --prefix=nginx --add-module=modules/nginx-let-module --with-pcre

make -j8
