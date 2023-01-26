// SPDX-License-Identifier: GPL-3.0-only

/*
 * eeg.c - Implementation of EEG data acquisition methods.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <med/eeg_priv.h>

#include "drivers.h"

int med_eeg_create(struct med_eeg **dev, char *type, struct med_kv *kv)
{
	/* TODO: This could use some link time array magic to let sources
	 * insert themselves. */

	if (!strcmp(type, "dummy"))
		return dummy_create(dev, kv);

	return -1;
}

void med_eeg_destroy(struct med_eeg *dev)
{
	struct med_sample *next;

	assert(dev);

	while (dev->samples) {
		next = dev->samples;
		dev->samples = next->next;
		free(next);
	}

	if (dev->destroy)
		dev->destroy(dev);
}

int med_eeg_set_mode(struct med_eeg *dev, enum med_eeg_mode mode)
{
	assert(dev);

	if (dev->set_mode)
		return dev->set_mode(dev, mode);

	return -1;
}

int med_eeg_get_channels(struct med_eeg *dev, char ***labels)
{
	assert(dev);

	if (dev->get_channels)
		return dev->get_channels(dev, labels);

	return -1;
}

int med_eeg_sample(struct med_eeg *dev, float *samples, int count)
{
	struct med_sample *next;
	int ret, i;

	assert(dev);

	if (!dev->sample)
		return -1;

	while (dev->sample_count < count) {
		ret = dev->sample(dev);
		if (ret < 0)
			return ret;
	}

	for (i = 0; i < count; ++i) {
		next = dev->samples;
		memcpy(samples, next->data, next->len * sizeof(next->data[0]));
		samples += next->len;
		dev->samples = next->next;
		free(next);
		dev->sample_count--;
	}

	return count;
}

int med_eeg_get_impedance(struct med_eeg *dev, float *samples)
{
	assert(dev);

	if (dev->get_impedance)
		return dev->get_impedance(dev, samples);

	return -1;
}


