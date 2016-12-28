#!/bin/bash

auto/configure --crossbuild=QNX:6.5.0:x86 --with-cc="i486-pc-nto-qnx6.5.0-gcc" --with-ld-opt="-lsocket" --prefix=nginx --add-module=modules/nginx-let-module --with-pcre

make -j8
