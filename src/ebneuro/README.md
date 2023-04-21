EBNeuro driver
==============

This driver implements the interface for the EBNeuro BE LTM Plus device.

Configuration
-------------

* `address` - IP address of the device (Mandatory)
* `packet_rate` - Amount of packets to send per second (default: 64)
* `data_rate` - Amount of samples to collect per second (default: 512)


Usage
-----

The driver will create 64 EEG channels named `eeg0` ... `eeg63` and 4 DC channels
named `dc0` ... `dc3`. Driver supports data, impedance and test modes. Note that
impedance values are cached on the device and updated once per second or so, which
means that requesting impedance data more often than that will return same values.
