# Deploying
## What does this page talk about?
This page will cover the steps to creating your own bare-metal environment for messing around with the Cobalt operating system.
## So, how do I get started?
To get started, you will need to build the toolchain. If you're on linux, just make sure that you have GCC (GNU Compiler Collection), and GNU make, aswell as some other tools which most likely already come with your linux installation. Then, you'll need GRUB2 installed, and xorriso installed aswell. Then you can run the following command:
```sh
make install-toolchain
```
After that, the root of your user directory will have a directory named `cobalt-toolchain`. Add it to PATH by using the command:
```sh
export PATH="/home/USERNAME/cobalt-toolchain/bin:$PATH"
```
You can also append that to the `.bashrc` file in the root of your user directory if you are using BASH. If you appended the above command to your `.bashrc` file, make sure to get a new instance of BASH running, or refresh it (if it's even possible).
Now you can just run this command:
```sh
make
```
This will build the Cobalt operating system. If you want it to build with ports, make sure to run `make install-ports` and then run `make` again.
To quickly test it with an emulator, you can use QEMU:
```sh
make qemu
```
Make sure you have QEMU installed first, or the emulation just won't work.
When Cobalt finishes building, it will produce a file named `cobalt.iso`. This is the CD-ROM image that can be flashed to a USB drive or a CD/DVD, for booting on an actual computer.
**These instructions will be clarified more in the future, if anyone is having trouble understanding these steps...**
