// SPDX-License-Identifier: GPL-3.0-only

/*
 * openbci.c - Driver for OpenBCI native communication protocol.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <med/eeg_priv.h>


static int openbci_sample(struct med_eeg *dev)
{
	return -1;
}

static int openbci_get_impedance(struct med_eeg *dev, float *samples)
{
	return -1;
}

static int openbci_set_mode(struct med_eeg *dev, enum med_eeg_mode mode)
{
	return -1;
}

static void openbci_destroy(struct med_eeg *dev)
{
	int i;

	for (i = 0; i < dev->channel_count; ++i)
		free(dev->channel_labels[i]);

	free(dev->channel_labels);
	free(dev);
}

int openbci_create(struct med_eeg **dev, struct med_kv *kv)
{
	int i, chan_cnt = 0;
	const char *key, *val;

	(*dev) = malloc(sizeof(**dev));
	memset(*dev, 0, sizeof(**dev));

	(*dev)->type          = "openbci";

	med_for_each_kv(kv, key, val) {
		med_dbg(*dev, "Parsing %s=%s", key, val);
	}

	(*dev)->channel_count  = chan_cnt;
	(*dev)->channel_labels = malloc(sizeof(char**) * chan_cnt);

	for (i = 0; i < chan_cnt; ++i) {
		(*dev)->channel_labels[i] = malloc(sizeof(char) * 8);
		snprintf((*dev)->channel_labels[i], 8, "sin%d", i);
	}

	(*dev)->sample         = openbci_sample;
	(*dev)->get_impedance  = openbci_get_impedance;
	(*dev)->set_mode       = openbci_set_mode;
	(*dev)->destroy        = openbci_destroy;
	
	return 0;
}
