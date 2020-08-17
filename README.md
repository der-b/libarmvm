# libarmvm

*libarmvm* is a software implementation of an Armv6-M processor.

## Build

``` bash
make
```

## Project structure

- *lib/* contains to source code of the libarmvm.
- *include/* contains the public interface to libarmvm.
- *arm-vm/* contains a command line interface (*arm-vm*) which is a wrapper for libarmvm.
- *example_programs/* contains several programs which can be loaded into the virtual machine and are used for testing.
- *test/* contains the unit tests.
