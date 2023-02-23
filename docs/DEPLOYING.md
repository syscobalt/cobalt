# Deploying
## What does this page talk about?
This page will cover the steps to creating your own bare-metal environment for messing around with the Cobalt operating system.
## So, how do I get started?
First of all, get a USB storage device (~4GiB is recommended). And then, split the USB drive in two by making two partitions. The first one should be around 1GiB, and will store the bootable non-persistent CD-ROM image of Cobalt. This partition is to be booted on a computer. The second partition is to be as big as the rest of the storage device, which should be a persistent ext4 filesystem, where you can write code that can be saved. For now, this is just the theory of a good way of playing around with the Cobalt operating system on bare metal, but later this page will contain actual instructions on how to set up this environment.
