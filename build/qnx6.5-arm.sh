#!/bin/bash
CFLAGS="-O3 -ffunction-sections -fdata-sections -funroll-loops"
auto/configure --crossbuild=QNX:6.5.0:arm \
--with-cc="arm-unknown-nto-qnx6.5.0-gcc" \
--with-ld-opt="-lsocket" \
--prefix=nginx \
--add-module=modules/nginx-let-module \
--with-http_v2_module \
--with-http_ssl_module

make -j8 install
