libmed - Data acquisition library for medical sensors
=====================================================

libmed is a low footprint library that implements interfaces of various medical
sensors such as EEG devices. The goal is to provide a portable library with a
reasonable abstraction layer over multiple device drivers.

The library provides a generic interface to acquire data from supported EEG
devices.

## Supported devices:
* Dummy driver - `dummy`
* EB Neuro BE Plus LTM - `ebneuro`
* OpenBCI Cyton - `openbci`

See documentation files in the driver directories.

## Build instructions:

libmed uses cmake as it's build system. This means the usual process is used:

```
mkdir build
cd build
cmake ..
make
```

If you want to use Python bindings for this library, you can use

```
pip3 install .
```

To build and install the library with the bindings.

