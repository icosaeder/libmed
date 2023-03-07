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

	int impedance_samples;

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
int obci_enable_channel(struct obci_dev *dev, int chan);
int obci_disable_channel(struct obci_dev *dev, int chan);
int obci_enable_test_signal(struct obci_dev *dev, char mode);
int obci_set_channel_config(struct obci_dev *dev, int chan, bool powerdown,
		int gain, char input_type, bool bias, bool srb2, bool srb1);
int obci_restore_defaults(struct obci_dev *dev);
int obci_set_leadoff_impedance(struct obci_dev *dev, int chan, bool pchan, bool nchan);
int obci_set_leadoff_impedance_all(struct obci_dev *dev, bool pchan, bool nchan);

#endif /* OPENBCI_H */
