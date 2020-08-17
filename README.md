# PitfulVM

A shabby implementation of Java virtual machine in C.

## Prerequisites

Java SE or OpenJDK is required to build and validate test suite.

## Running the tests

You can run the tests with `make check`.

## Running the VM

You need to specify the full filename to the executable. For example:
```shell
$ javac tests/Factorial.java
$ ./jvm tests/Factorial.class
```

## License

`PitfulVM` is released under the MIT License. Use of this source code is
governed by a MIT License that can be found in the LICENSE file.

