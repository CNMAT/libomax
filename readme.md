# libomax

libomax is an interoperability layer between
[https://github.com/CNMAT/libo](libo), and Max/MSP and PD,
by [https://github.com/maccallum](John MacCallum).

## Building libomax and libopd

libomax can be built on Mac OS X, Windows 10 (MinGW + MSYS2), and many flavors
of Linux, including Raspberry Pi OS. Building is done against the Max
and/or PD SDKs, and results in the static libraries `libomax.a` and `libopd.a`,
which are necessary to build the odot objects.

### Prerequisites for all platforms

* You need the appropriate SDK(s) for the platform you want to target:
	* Max: [https://github.com/cycling74/max-sdk](https://github.com/cycling74/max-sdk)
	* PD: [https://github.com/pure-data/pure-data](https://github.com/pure-data/pure-data)
* [https://github.com/CNMAT/libo](https://github.com/CNMAT/libo)
	* See the libo documentation for build instructions
* The libomax folder should be in the same folder that also contains libo,
  and the Max and/or PD SDK(s).

### Mac OS X

```
$ cd <path/to/libomax>
$ make
```

### Windows 10

The Windows 10 build is done using MinGW under MSYS2. Make sure you have
installed the 64-bit GCC toolchain (`pacman -S mingw-w64-x86_64-toolchain`).

1. Build PD / libpd (skip this step if you're not building for PD). Follow the
instructions in the pure-data source---it should be something like

```
$ cd <path/to/pure-data>
$ ./autogen.sh
$ ./configure
$ make
```

2. Build libomax / libopd
```
$ cd <path/to/libomax>
$ make win64
```

Note that 32-bit builds on Windows are not supported.

### Linux

The Makefile is set up to use clang. GCC should also work, with the appropriate
changes to the Makefile.

Note that PD / libpd does not need to be built first.

```
$ cd <path/to/libomax>
$ make linux
```