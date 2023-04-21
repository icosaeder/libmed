OpenBCI driver
==============

This driver implements support for OpenBCI Cyton board. 

Configuration
-------------

* `port` - Serial port of the device (Mandatory)
* `channels` - Amount of channels the board has. (Default: 16)
* `gain` - ADC Gain (default: 24)
* `impedance_samples` - Amount of samples to use to calculate the impedance (Default: 30)


Usage
-----

The driver provides 8 or 16 EEG channels named `eeg0`...`eegN` depending on whether
the Daisy subboard is installed. Please note that `channels` must be set to 8
explicitly if the subboard is absent.
