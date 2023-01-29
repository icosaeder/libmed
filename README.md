libmed - Data acquisition library for medical sensors
=====================================================

libmed is a low footprint library that implements interfaces of various medical
sensors such as EEG devices. The goal is to provide a portable library with a
reasonable abstraction layer over multiple device drivers.

The library is in the early development so it *does not* provide any generic
interface at this time but is just a collection of implementations.
The interface stability is *not guaranteed*.

## Supported devices:
* Dummy driver - `dummy`
* EB Neuro BE Plus LTM - `ebneuro`

## Build instructions:

libmed uses cmake as it's build system. This means the usual process is used:

```
mkdir build
cd build
cmake ..
make
```

