// SPDX-License-Identifier: GPL-3.0-only

/*
 * dummy.c - Dummy EEG device implementation.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <med/eeg.h>

#define CHAN_CNT 4

static int dummy_sample(struct med_eeg *dev)
{
	struct med_sample *next = malloc(sizeof(*next) + sizeof(float) * CHAN_CNT);
	static float v=1;
	int i;

	for (i = 0; i < CHAN_CNT; ++i) {
		next->data[i] = v+=0.1;
	}

	next->len = CHAN_CNT;
	next->next = dev->samples;
	dev->samples = next;
	dev->sample_count++;

	return 1;
}

int dummy_create(struct med_eeg **dev, struct med_kv *kv)
{
	(*dev) = malloc(sizeof(**dev));
	memset(*dev, 0, sizeof(**dev));

	(*dev)->channel_count = CHAN_CNT;
	(*dev)->sample = dummy_sample;
	(*dev)->destroy = free;
	
	return 0;
}
