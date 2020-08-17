# PitifulVM

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

`PitifulVM` is released under the BSD 2 clause license. Use of this source code
is governed by a BSD-style license that can be found in the LICENSE file.
