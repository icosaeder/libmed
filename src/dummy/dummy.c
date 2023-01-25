// SPDX-License-Identifier: GPL-3.0-only

/*
 * dummy.c - Dummy EEG device implementation.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <med/eeg.h>

#define CHAN_CNT 4

static char *dummy_channel_labels[] = {
	"sin0",
	"sin1",
	"sin2",
	"sin3",
};

static int dummy_get_channels(struct med_eeg *dev, char ***labels)
{
	*labels = dummy_channel_labels;

	return dev->channel_count;
}

static int dummy_sample(struct med_eeg *dev)
{
	struct med_sample *next;
	static float v=1;
	int i;

	next = malloc(sizeof(*next) + sizeof(float) * dev->channel_count);

	for (i = 0; i < dev->channel_count; ++i)
		next->data[i] = 1. + sin(v+=0.1);

	next->len = dev->channel_count;
	next->next = dev->samples;
	dev->samples = next;
	dev->sample_count++;

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
	return 0;
}

static void dummy_destroy(struct med_eeg *dev)
{
	free(dev);
}

int dummy_create(struct med_eeg **dev, struct med_kv *kv)
{
	(*dev) = malloc(sizeof(**dev));
	memset(*dev, 0, sizeof(**dev));

	(*dev)->channel_count = CHAN_CNT;
	(*dev)->sample        = dummy_sample;
	(*dev)->get_impedance = dummy_get_impedance;
	(*dev)->get_channels  = dummy_get_channels;
	(*dev)->set_mode      = dummy_set_mode;
	(*dev)->destroy       = dummy_destroy;
	
	return 0;
}
