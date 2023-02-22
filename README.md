---
File: "README.md"
Author: "Falkosc223, Dashbloxx"
---
# Cobalt
## What is Cobalt?
Cobalt is a UNIX-like operating system that is designed for speed, and compability. Cobalt is nearly compatible with most programs made for UNIX-like systems. Cobalt contains code from Cobalt & Sortix.
## How can I build Cobalt?
To build Cobalt, you will need to build the toolchain first. Start by running the command:
```
make install-toolchain
```
This will build the toolchain required for building Cobalt and it's applications. You will need to add the built toolchain to your PATH, in order to then use it. Then, you can build the rest:
```
make distclean
make all
```
The first line will clean the root filesystem model, and delete the `.iso` file. The second line builds Cobalt from source.
## Testing Cobalt
Cobalt currently supports only two architectures, which are i386, and x86_64. When Cobalt is built for one of these architectures you can test them using QEMU:
```
qemu-system-x86_64 -m 1024M -cdrom cobalt.iso
```
This will get QEMU to test out the CDROM image of Cobalt that was built. If you are running on a x86_64 architecture, you can add `-accel kvm` to make the emulation even faster.
## Contributing
For guidelines on contributing, see [this](CONTRIBUTING.md) page. A list of contributors can be found here:
* [Falkosc223](https://github.com/orgs/syscobalt/people/Falkosc223)
* [Dashbloxx](https://github.com/orgs/syscobalt/people/Dashbloxx)

Others who created code which can be found here are listed aswell:
* [dennis95](https://github.com/dennis95)
## Ports
There is a subfolder called `ports`, which uses a port manager made for [dennix](https://github.com/dennis95/dennix) that is compatible with Cobalt. Most of these ports have been made for Dennix, but were slightly modified to work with Cobalt.
These ports are automatically built when you run `make`, because they are considered vital, although others may see it as bloatware. Cobalt itself is very fast, but with the basic ports like GCC, AWK, BIM, binutils, DOOM, gettext, libSDL, some other basic libraries, make, nano, ncurses being loaded to RAM at startup (the initial ramdisk stores these binaries!), it makes the boot process much slower. This however, will be fixed in the future, as Cobalt will install itself to the disk and allow you to load binaries to RAM right when you need to run them.
