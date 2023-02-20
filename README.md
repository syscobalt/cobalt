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
Later if you want to test Cobalt, just run:
```
qemu-system-x86_64 -cdrom cobalt.iso
```
If you're using linux on a x86_64 machine, you can run:
```
qemu-system-x86_64 -accel kvm -cdrom cobalt.iso
```
This will test Cobalt a bit faster.