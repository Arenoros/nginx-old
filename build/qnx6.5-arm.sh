#!/bin/bash

 auto/configure --crossbuild=QNX:6.5.0:arm --with-cc="arm-unknown-nto-qnx6.5.0-gcc" --with-ld-opt="-lsocket" --prefix=nginx --add-module=modules/nginx-let-module --with-pcre --with-http_v2_module --with-http_ssl_module --with-pcre-jit

make -j8
