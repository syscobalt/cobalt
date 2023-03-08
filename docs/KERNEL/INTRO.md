# Introduction
The kernel is one of the most essential parts of the Cobalt operating system (well pretty much the same for all other operating systems). The kernel runs in a full-privilege mode. The kernel provides syscalls and sets up and/or messes with hardware.
The kernel is loaded by the GRUB bootloader (it most likely can be launched by other bootloaders aswell!), sets up drivers and hardware, and then loads the initial ramdisk into the UNIX-like filesystem. Then, the `init` binary gets started.
This directory will contain different files explaining exactly what the kernel does, and it will also document the code in the kernel.
