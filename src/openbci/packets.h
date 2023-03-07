/* SPDX-License-Identifier: GPL-3.0-only */

#ifndef OPENBCI_PACKETS_H
#define OPENBCI_PACKETS_H

#include <stdint.h>
#include <assert.h>

#include "OpenBCI_32bit_Library_Definitions.h"

struct openbci_data {
	uint8_t magic;
	uint8_t seq;
	uint8_t data[OPENBCI_NUMBER_BYTES_PER_ADS_SAMPLE];
	uint8_t aux[OPENBCI_NUMBER_OF_BYTES_AUX];
	uint8_t stop;
};

#define OPENBCI_DATA_MAGIC 0xa0
#define OPENBCI_DATA_END_MAGIC 0xc0

/*
 * Some ectra helpers on top of the imported protocol definitions.
 */

#define OPENBCI_CHANNEL_OFF(chan) \
	(chan < 9 ? (OPENBCI_CHANNEL_OFF_1 + chan - 1) : (OPENBCI_CHANNEL_OFF_9 + chan - 8))
#define OPENBCI_CHANNEL_ON(chan) \
	(chan < 9 ? (OPENBCI_CHANNEL_ON_1 + chan - 1) : (OPENBCI_CHANNEL_ON_9 + chan - 8))

#define OPENBCI_CHANNEL_CMD_CHANNEL(chan) \
	(chan < 9 ? ('1' + chan - 1) : ('Q' + chan - 8))

#define OPENBCI_CHANNEL_CMD_GAIN(gain) ( \
		gain < 2  ? OPENBCI_CHANNEL_CMD_GAIN_1  : \
		gain < 4  ? OPENBCI_CHANNEL_CMD_GAIN_2  : \
		gain < 6  ? OPENBCI_CHANNEL_CMD_GAIN_4  : \
		gain < 8  ? OPENBCI_CHANNEL_CMD_GAIN_6  : \
		gain < 12 ? OPENBCI_CHANNEL_CMD_GAIN_8  : \
		gain < 24 ? OPENBCI_CHANNEL_CMD_GAIN_12 : \
		OPENBCI_CHANNEL_CMD_GAIN_24 )



#endif /* OPENBCI_PACKETS_H */

