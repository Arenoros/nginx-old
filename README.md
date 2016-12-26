# nginx
A cross-compilation of nginx.
Added cross compilation for 
 -- QNX 6.5.0(x86 | arm)
 -- Linux arm

#### Prerequisite

- QNX SDP 6.x
- gcc-arm-linux-gnueabi or gcc-arm-linux-gnueabihf

#### Building for QNX

2. Run `auto/configure` with the following parameters (you can add more parameters as you need):

  for x86:

    - --crossbuild=QNX:6.5.0:x86
    - --with-cc="qcc -V4.4.2,gcc_ntox86"
    - --with-ld-opt="-lsocket"

  for arm:

    - --crossbuild=QNX:6.5.0:arm
    - --with-cc="qcc -V4.4.2,gcc_ntoarmv7le"
    - --with-ld-opt="-lsocket"

3. Run make

    $ make

#### Building for Linux / arm

1. Run `configure` with the following parameters (you can add more parameters as you need):

    - --crossbuild=Linux::arm
    - --with-cc="arm-linux-gnueabihf-gcc"
    - --with-ld-opt="-L/usr/arm-linux-gnueabihf/lib"
    - --with-cc-opt="-L/usr/arm-linux-gnueabihf/include"

 For example (in case you installed arm-linux-gnueabihf):
in case you installed arm-linux-gnueabihf

  $ ./configure --crossbuild=Linux::arm --with-cc="arm-linux-gnueabihf-gcc" --with-ld-opt="-L/usr/arm-linux-gnueabihf/lib" --with-cc-opt="-L/usr/arm-linux-gnueabihf/include"

2. Run make

    $ make
