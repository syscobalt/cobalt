---
File: "README.md"
Author: "Falkosc223, Dashbloxx"
---
# Cobalt
## What is Cobalt?
Cobalt is a UNIX-like operating system that is designed for speed, and compability. Cobalt is nearly compatible with most programs made for UNIX-like systems. Cobalt contains code from Dennix & Sortix.
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
qemu-system-x86_64 -cdrom cobalt.iso
```
This will get QEMU to test out the CDROM image of Cobalt that was built.
## Contributing
For guidelines on contributing, see [this](CONTRIBUTING.md) page.