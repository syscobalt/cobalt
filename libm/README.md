# Mathematics library (libm)
## What is this?
This is the mathematics library stripped out of musl-libc. This library was then modified to work with Cobalt, and was retained when Cobalt was forked from Cobalt.
Here are some notes from the old README file:
```
The current version is based on commit d91a6cf6e369a79587c5665fce9635e5634ca201
of musl.
It is licensed under the MIT license compatible to the ISC license used
by Cobalt. Some code is licensed under other permissive terms as noted in the
individual files.
The math library is not built as a separate libm.a libary but is built is part
of libc. An empty libm.a file is installed so that programs that try to link
with libm will work.
```