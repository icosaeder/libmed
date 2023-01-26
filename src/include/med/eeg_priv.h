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
 * @type:          Type of the device.
 * @channel_count: Amount of channels in the sample.
 */
struct med_eeg {
	char *type;
	int channel_count;

	int sample_count;
	struct med_sample *samples;

	/* Unprepare and destroy the resources. */
	void (*destroy)(struct med_eeg *dev);
	
	/* Set the device mode. */
	int (*set_mode)(struct med_eeg *dev, enum med_eeg_mode mode);
	/* Get available channels */
	int (*get_channels)(struct med_eeg *dev, char ***labels);
	/* Read currently available samples into the sample buffer. */
	int (*sample)(struct med_eeg *dev);
	/* Read out impedance on all possible cahnnels to the user. */
	int (*get_impedance)(struct med_eeg *dev, float *samples);
};

/* debug print helpers */
#define med_err(dev, fmt, ...) \
	s_dprintf(CRITICAL, "[%s] %s:%d: " fmt "\n", \
			dev->type, __func__, __LINE__, ##__VA_ARGS__)

#define med_info(dev, fmt, ...) \
	s_dprintf(INFO, "[%s] %s:%d: " fmt "\n", \
			dev->type, __func__, __LINE__, ##__VA_ARGS__)

#define med_dbg(dev, fmt, ...) \
	s_dprintf(SPEW, "[%s] %s:%d: " fmt "\n", \
			dev->type, __func__, __LINE__, ##__VA_ARGS__)

#endif /* EEG_PRIV_H */
