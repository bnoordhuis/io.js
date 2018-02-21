## cross-compiling

Cross-compiling arch -> arch for the same distro + release should, in theory,
just work, if that distro is a recent enough Debian or Ubuntu.  Let's assume
we want to target gnueabihf:

    $ sudo apt-get install binfmt-support qemu-user g++-arm-linux-gnueabihf

    # Tell qemu where to look for shared libraries.
    $ export QEMU_LD_PREFIX=/usr/arm-linux-gnueabihf

    $ mkdir -p build && cd build

    $ cmake -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=arm \
        -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc \
        -DCMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++ ..

    $ make -j4
