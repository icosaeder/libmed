// SPDX-License-Identifier: GPL-3.0-only

/*
 * dummy.c - Dummy EEG device implementation.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <med/eeg_priv.h>

#define CHAN_CNT 4

static int dummy_sample(struct med_eeg *dev)
{
	struct med_sample *next = med_eeg_alloc_sample(dev);
	static float v=1;
	int i;

	for (i = 0; i < dev->channel_count; ++i)
		next->data[i] = 1. + sin(v+=0.1);

	med_eeg_add_sample(dev, next);

	return 1;
}

static int dummy_get_impedance(struct med_eeg *dev, float *samples)
{
	int i;

	for (i = 0; i < dev->channel_count; ++i)
		samples[i] = (float)i;

	return dev->channel_count;
}

static int dummy_set_mode(struct med_eeg *dev, enum med_eeg_mode mode)
{
	med_dbg(dev, "Mode was set to %d", mode);

	return 0;
}

static void dummy_destroy(struct med_eeg *dev)
{
	int i;

	for (i = 0; i < dev->channel_count; ++i)
		free(dev->channel_labels[i]);

	free(dev->channel_labels);
	free(dev);
}

int dummy_create(struct med_eeg **dev, struct med_kv *kv)
{
	int i, chan_cnt = CHAN_CNT;
	const char *key, *val;

	(*dev) = malloc(sizeof(**dev));
	memset(*dev, 0, sizeof(**dev));

	(*dev)->type          = "dummy";

	med_for_each_kv(kv, key, val) {
		med_dbg(*dev, "Parsing %s=%s", key, val);

		if (!strcmp("channels", key))
			chan_cnt = atoi(val);
	}

	(*dev)->channel_count  = chan_cnt;
	(*dev)->channel_labels = malloc(sizeof(char**) * chan_cnt);

	for (i = 0; i < chan_cnt; ++i) {
		(*dev)->channel_labels[i] = malloc(sizeof(char) * 8);
		snprintf((*dev)->channel_labels[i], 8, "sin%d", i);
	}

	(*dev)->sample         = dummy_sample;
	(*dev)->get_impedance  = dummy_get_impedance;
	(*dev)->set_mode       = dummy_set_mode;
	(*dev)->destroy        = dummy_destroy;
	
	return 0;
}
