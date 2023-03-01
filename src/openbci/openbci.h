/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef OPENBCI_H
#define OPENBCI_H

#include <stdbool.h>

#include <med/eeg_priv.h>

#include "packets.h"

struct obci_dev {

	struct med_eeg edev;

	char *port;
	int fd;
	int baud_rate;

	bool is_streaming;

	float scratch[OPENBCI_ADS_CHANS_PER_BOARD];
};

/* packets.c */

int obci_text_cmd(struct obci_dev *dev, const char cmd, char *buf, size_t len);
int obci_text_cmds(struct obci_dev *dev, const char *cmd, char *buf, size_t len);
int obci_read_data_pkt(struct obci_dev *dev, struct openbci_data *data);

/* commands.c */

int obci_reset(struct obci_dev *dev);
int obci_get_version(struct obci_dev *dev);
int obci_get_max_channels(struct obci_dev *dev);
int obci_set_max_channels(struct obci_dev *dev, int channels);
int obci_set_streaming(struct obci_dev *dev, bool streaming);


#endif /* OPENBCI_H */
