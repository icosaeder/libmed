/* SPDX-License-Identifier: GPL-3.0-only */

#ifndef OPENBCI_PACKETS_H
#define OPENBCI_PACKETS_H

#include <stdint.h>

#include "OpenBCI_32bit_Library_Definitions.h"

struct openbci_data {
	uint8_t magic;
	uint8_t seq;
	uint8_t data[OPENBCI_NUMBER_BYTES_PER_ADS_SAMPLE];
	uint8_t aux[OPENBCI_NUMBER_OF_BYTES_AUX];
	uint8_t stop;
};

#define OPENBCI_DATA_MAGIC 0xa0

#endif /* OPENBCI_PACKETS_H */

