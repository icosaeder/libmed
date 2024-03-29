/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef EEG_PRIV_H
#define EEG_PRIV_H

#include <system/system.h>
#include <med/eeg.h>

/**
 * struct med_sample - Internal sample storage.
 */
struct med_sample {
	struct med_sample *next;
	int seq;
	int len;
	float data[];
};

/**
 * struct med_eeg - EEG device.
 * @type:           Type of the device.
 * @channel_count:  Amount of channels in the sample.
 * @channel_labels: An array of labels for the channels.
 * @sample_count:   Ammount of ready samples.
 * @samples:        A list of already acquired samples.
 * @samples_tail:   The end of the sample list to append to.
 * @destroy:        Unprepare and destroy the resources.
 * @set_mode:       Set the device mode.
 * @sample:         Read currently available samples into the sample buffer.
 * @get_impedance:  Read out impedance on all possible cahnnels to the user.
 */
struct med_eeg {
	char *type;

	int channel_count;
	char **channel_labels;

	int sample_count;
	struct med_sample *samples;
	struct med_sample *samples_tail;

	void (*destroy)(struct med_eeg *dev);
	int (*set_mode)(struct med_eeg *dev, enum med_eeg_mode mode);
	int (*sample)(struct med_eeg *dev);
	int (*get_impedance)(struct med_eeg *dev, float *samples);
};

/**
 * med_eeg_alloc_sample() - Allocate a sample
 */
static inline struct med_sample *med_eeg_alloc_sample(struct med_eeg *dev)
{
	struct med_sample *next;

	next = malloc(sizeof(*next) + sizeof(float) * dev->channel_count);
	next->len = dev->channel_count;
	next->next = NULL;

	return next;
}

/**
 * med_eeg_add_sample() - Insert the newly created sample to the queue.
 */
static inline void med_eeg_add_sample(struct med_eeg *dev, struct med_sample *next)
{
	if (!dev->samples)
		dev->samples = next;
	else
		dev->samples_tail->next = next;

	dev->samples_tail = next;
	dev->sample_count++;
}

/* debug print helpers */
#define med_err(dev, fmt, ...) \
	s_dprintf(CRITICAL, "[%s] %s:%d: " fmt "\n", \
			(dev)->type, __func__, __LINE__, ##__VA_ARGS__)

#define med_info(dev, fmt, ...) \
	s_dprintf(INFO, "[%s] %s:%d: " fmt "\n", \
			(dev)->type, __func__, __LINE__, ##__VA_ARGS__)

#define med_dbg(dev, fmt, ...) \
	s_dprintf(SPEW, "[%s] %s:%d: " fmt "\n", \
			(dev)->type, __func__, __LINE__, ##__VA_ARGS__)

#define med_for_each_kv(_kv, _key, _val) \
	for (_key=(_kv ? _kv->key : NULL), _val=(_kv ? _kv->val : NULL); \
		_kv && _kv->key; \
		_kv++, _key=_kv->key, _val=_kv->val)

#endif /* EEG_PRIV_H */
