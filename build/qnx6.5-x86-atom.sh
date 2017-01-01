#!/bin/sh
export CFLAGS="-march=atom -mtune=atom -O3 -flto -ffast-math -fno-use-linker-plugin"
TMP_x86=
if [ -d lib/QNX/x86 ]; then
	TMP_x86="lib/QNX/tmp-arch"
	cp -R "lib/QNX/x86" $TMP_x86
fi
if [ -d lib/QNX/x86-atom ]; then
	rm -R "lib/QNX/x86"
	cp -R "lib/QNX/x86-atom" "lib/QNX/x86"
fi
auto/configure --crossbuild=QNX:6.5.0:x86 --with-cc="i486-pc-nto-qnx6.5.0-gcc" --with-ld-opt="-lsocket" --prefix=nginx --add-module=modules/nginx-let-module --with-pcre

make -j8

if [ ! -z "$TMP_x86" ]; then
	cp -R $TMP_x86 "lib/QNX/x86"
	rm -R $TMP_x86
fi
export CFLAGS=
