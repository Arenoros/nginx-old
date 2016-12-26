#!/bin/bash

auto/configure --prefix=nginx --add-module=modules/nginx-let-module --with-pcre

make -j8
