// SPDX-License-Identifier: GPL-3.0-only

/*
 * eeg.c - Implementation of EEG data acquisition methods.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <system/system.h>
#include <med/eeg_priv.h>

#include "drivers.h"

int med_eeg_create(struct med_eeg **dev, char *type, struct med_kv *kv)
{
	struct med_kv *ckv = kv;
	char *key, *val;

	med_for_each_kv(ckv, key, val) {
		if (!strcmp(key, "verbosity"))
			s_set_verbosity(atoi(val));
	}

	if (!strcmp(type, "dummy"))
		return dummy_create(dev, kv);
	if (!strcmp(type, "ebneuro"))
		return ebneuro_create(dev, kv);

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

	if (labels)
		*labels = dev->channel_labels;

	return dev->channel_count;
}

int med_eeg_sample(struct med_eeg *dev, float *samples, int count)
{
	struct med_sample *next;
	int ret, i;

	assert(dev);

	if (!dev->sample)
		return -1;

	do {
		ret = dev->sample(dev);
		if (ret < 0)
			return ret;
	} while (dev->sample_count < count);

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


