// SPDX-License-Identifier: GPL-3.0-only

/*
 * openbci.c - Driver for OpenBCI native communication protocol.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <errno.h>
#include <unistd.h>
#include <termios.h>

#include <med/eeg_priv.h>
#include <system/system.h>
#include <system/helpers.h>

#include "openbci.h"
#include "packets.h"
#include "OpenBCI_32bit_Library_Definitions.h"

static int i24to32(uint8_t *bytes) {
	int ret = (bytes[0] << 16) | (bytes[1] << 8) | bytes[2];

	if (ret & 0x00800000)
		ret |= 0xFF000000;
	else
		ret &= 0x00FFFFFF;

	return ret;
}

static int obci_read_sample(struct obci_dev *dev, float *samples)
{
	struct openbci_data data = {0};
	float tmp[OPENBCI_ADS_CHANS_PER_BOARD];
	int i, ret;

	ret = obci_read_data_pkt(dev, &data);
	if (ret < 0)
		return ret;

	for (i = 0; i < OPENBCI_ADS_CHANS_PER_BOARD; ++i)
		tmp[i] = (float)i24to32(&data.data[i*3]) * (4.5 / (2<<22 - 1)) / dev->gain / 2;

	if (dev->edev.channel_count == 8) {
		memcpy(samples, tmp, sizeof(tmp));
	} else if (data.seq & 1) {
		/* board */
		memcpy(samples, tmp, sizeof(tmp));
		memcpy(&samples[OPENBCI_ADS_CHANS_PER_BOARD], dev->scratch, sizeof(dev->scratch));
	} else {
		/* daisy */
		memcpy(&samples[OPENBCI_ADS_CHANS_PER_BOARD], tmp, sizeof(tmp));
		memcpy(samples, dev->scratch, sizeof(dev->scratch));
	}

	memcpy(dev->scratch, tmp, sizeof(tmp));

	return 0;
}

static int openbci_sample(struct med_eeg *edev)
{
	struct obci_dev *dev = container_of(edev, struct obci_dev, edev);
	struct med_sample *next = med_eeg_alloc_sample(edev);
	int ret;

	ret = obci_read_sample(dev, next->data);
	if (ret < 0) {
		free(next);
		return ret;
	}

	med_eeg_add_sample(edev, next);

	return 1;
}

static int openbci_get_impedance(struct med_eeg *edev, float *samples)
{
	struct obci_dev *dev = container_of(edev, struct obci_dev, edev);
	float *buf = malloc(sizeof(float) * edev->channel_count * dev->impedance_samples);
	int i, ret;

	for (i = 0; i < dev->impedance_samples; i++) {
		ret = obci_read_sample(dev, &buf[edev->channel_count * i]);
		if (ret < 0)
			goto error;
	}

	ret = obci_calculate_leadoff_impedane(buf, samples, dev->impedance_samples, edev->channel_count);

error:
	free(buf);
	return ret;
}

static int openbci_set_mode(struct med_eeg *edev, enum med_eeg_mode mode)
{
	struct obci_dev *dev = container_of(edev, struct obci_dev, edev);
	char buf[128];
	int ret;

	switch (mode) {
	case MED_EEG_IDLE:
		return obci_set_streaming(dev, false);

	case MED_EEG_SAMPLING:
		ret = obci_restore_defaults(dev);
		if (ret < 0)
			return ret;

		ret = obci_set_channel_config_all(dev, false, dev->gain, OPENBCI_CHANNEL_CMD_ADC_Normal, false, true, false);
		if (ret < 0)
			return ret;

		ret = obci_set_leadoff_impedance(dev, 0, false, true);

		return obci_set_streaming(dev, true);

	case MED_EEG_IMPEDANCE:
		ret = obci_set_channel_config(dev, 0, false, dev->gain, OPENBCI_CHANNEL_CMD_ADC_Normal, true, true, false);
		if (ret < 0)
			return ret;

		ret = obci_set_leadoff_impedance(dev, 0, false, true);
		if (ret < 0)
			return ret;

		return obci_set_streaming(dev, true);

	case MED_EEG_TEST:
		ret = obci_enable_test_signal(dev, OPENBCI_TEST_SIGNAL_CONNECT_TO_PULSE_1X_FAST);
		if (ret < 0)
			return ret;

		return obci_set_streaming(dev, true);
	}
}

static void openbci_destroy(struct med_eeg *edev)
{
	struct obci_dev *dev = container_of(edev, struct obci_dev, edev);
	int i;

	obci_reset(dev);

	close(dev->fd);

	free(dev->port);

	for (i = 0; i < edev->channel_count; ++i)
		free(edev->channel_labels[i]);

	free(edev->channel_labels);
	free(dev);
}

static int obci_init(struct obci_dev *dev)
{
	unsigned char tmp;
	int ret; 

	ret = obci_reset(dev);
	if (ret < 0)
		return ret;

	ret = obci_get_version(dev);
	if (ret < 0) {
		med_err(&dev->edev, "Failed to read firmware version: %d", ret);
		med_err(&dev->edev, "Is the device firmware up to date?");
		return ret;
	}

	return 0;
	/*
	 * FIXME: Firmware seems to be borked so touching the subboard config
	 * fails miserably...
	 */

	if (dev->edev.channel_count) {
		ret = obci_set_max_channels(dev, dev->edev.channel_count);
		if (ret < 0)
			return ret;
	} else {
		ret = obci_get_max_channels(dev);
		if (ret < 0)
			return ret;

		dev->edev.channel_count = ret;
	}

	return 0;
}

int openbci_create(struct med_eeg **edev, struct med_kv *kv)
{
	struct obci_dev *dev = malloc(sizeof(*dev));
	const char *key, *val;
	int ret, i, chan_cnt = 0;

	memset(dev, 0, sizeof(*dev));

	(*edev) = &dev->edev;
	(*edev)->type           = "openbci";
	(*edev)->channel_count  = 16;

	dev->baud_rate = B115200;
	dev->impedance_samples  = 30;
	dev->gain               = 24;

	med_for_each_kv(kv, key, val) {
		med_dbg(*edev, "Parsing %s=%s", key, val);

		if (!strcmp("port", key))
			dev->port = strdup(val);
		else if (!strcmp("channels", key))
			(*edev)->channel_count = atoi(val);
		else if (!strcmp("impedance_samples", key))
			dev->impedance_samples = atoi(val);
		else if (!strcmp("gain", key))
			dev->gain = atoi(val);
	}

	dev->gain = OPENBCI_CLAMP_GAIN(dev->gain);

	ret = s_serial(&(dev->fd), dev->port, dev->baud_rate, 0);
	if (ret < 0)
		return ret;

	med_dbg(&dev->edev, "Opening port %s.\n", dev->port);

	ret = obci_init(dev);
	if (ret < 0)
		return ret;

	chan_cnt = (*edev)->channel_count;

	(*edev)->channel_labels = malloc(sizeof(char**) * chan_cnt);

	for (i = 0; i < chan_cnt; ++i) {
		(*edev)->channel_labels[i] = malloc(sizeof(char) * 8);
		snprintf((*edev)->channel_labels[i], 8, "eeg%d", i);
	}

	(*edev)->sample         = openbci_sample;
	(*edev)->get_impedance  = openbci_get_impedance;
	(*edev)->set_mode       = openbci_set_mode;
	(*edev)->destroy        = openbci_destroy;
	
	return 0;
}
