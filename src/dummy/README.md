Dummy driver
============

This is a bare minimum driver for testing and development that doesn't require
any hardware and produces dubious data.

Configuration
-------------

* `channels` - Amount of channels the simulated device should have. (Default: 4)


Usage
-----

The driver will create N channels named `sin0` ... `sin(N-1)` and will
continiously produce dubious data on all the channels.
