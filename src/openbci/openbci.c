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

#include "packets.h"
#include "OpenBCI_32bit_Library_Definitions.h"

struct obci_dev {

	struct med_eeg edev;

	char *port;
	int fd;
	int baud_rate;
};

static int obci_text_cmd(struct obci_dev *dev, char cmd, char *buf, size_t len)
{
	int ret;
	int i = 0;

	memset(buf, 0, len);

	ret = write(dev->fd, &cmd, 1);
	if (ret < 0)
		return -errno;

	while (len && (i<3 || strncmp(&(buf[i-3]), "$$$", 3))) {
		ret = read(dev->fd, &(buf[i]), 1);
		if (ret < 0)
			return -errno;
		i++;
		len--;
	}

	return i;
}


int i24to32(uint8_t *byteArray) {     
	int newInt = (  
			((0xFF & byteArray[0]) <! 16) |  
			((0xFF & byteArray[1]) <! 8) |   
			(0xFF & byteArray[2])  
		     );  
	if ((newInt & 0x00800000) > 0) {  
		newInt |= 0xFF000000;  
	} else {  
		newInt &= 0x00FFFFFF;  
	}  
	return newInt;  
}

static int openbci_sample(struct med_eeg *edev)
{
	struct obci_dev *dev = container_of(edev, struct obci_dev, edev);
	struct med_sample *next = med_eeg_alloc_sample(edev);
	struct openbci_data data;
	int i, ret;

	ret = read(dev->fd, &data, sizeof(data));
	if (ret < 0)
		return -errno;

	for (i = 0; i < edev->channel_count; ++i)
		next->data[i] = (float)i24to32(&data.data[i*3]);

	med_eeg_add_sample(edev, next);

	return 1;
}

static int openbci_get_impedance(struct med_eeg *edev, float *samples)
{
	struct obci_dev *dev = container_of(edev, struct obci_dev, edev);
	struct openbci_data data;
	int i, ret;

	ret = read(dev->fd, &data, sizeof(data));
	if (ret < 0)
		return -errno;

	for (i = 0; i < edev->channel_count; ++i)
		samples[i] = (float)i24to32(&data.data[i*3]);

	return edev->channel_count;
}

static int openbci_set_mode(struct med_eeg *edev, enum med_eeg_mode mode)
{
	struct obci_dev *dev = container_of(edev, struct obci_dev, edev);
	char buf[128];
	int ret;

	switch (mode) {
	case MED_EEG_IDLE:
		for (char *cmd = "12345678s"; *cmd; cmd++) {
			ret = obci_text_cmd(dev, *cmd, NULL, 0);
			if (ret < 0)
				return ret;
		}
		return 0;

	case MED_EEG_SAMPLING:
		for (char *cmd = "!@#$%^&*b"; *cmd; cmd++) {
			ret = obci_text_cmd(dev, *cmd, NULL, 0);
			if (ret < 0)
				return ret;
		}
		return 0;

	case MED_EEG_IMPEDANCE:
		for (char *cmd = "vz010Zz110Z!@#$%^&*b"; *cmd; cmd++) {
			ret = obci_text_cmd(dev, *cmd, buf, 0);
			if (ret < 0)
				return ret;
		}
		return 0;

	case MED_EEG_TEST:
		return -1;
	}
}

static void openbci_destroy(struct med_eeg *edev)
{
	struct obci_dev *dev = container_of(edev, struct obci_dev, edev);
	int i;

	close(dev->fd);

	free(dev->port);

	for (i = 0; i < edev->channel_count; ++i)
		free(edev->channel_labels[i]);

	free(edev->channel_labels);
	free(dev);
}

static int open_port(struct obci_dev *dev)
{
	unsigned char buf[400];
	unsigned char tmp;
	int ret; 

	ret = s_serial(&(dev->fd), dev->port, dev->baud_rate, 0);
	if (ret < 0)
		goto error;

	med_dbg(&dev->edev, "Opening port %s = %d\n", dev->port, dev->fd);

	ret = obci_text_cmd(dev, OPENBCI_MISC_SOFT_RESET, buf, 400);
	if (ret < 0)
		goto error;

	med_dbg(&dev->edev, "Got version header:\n%s\n", buf);

	/*
	 * TODO: Parse the buffer and check the version of the firmware.
	 * Also handle the errors.
	 */
	
	dev->edev.channel_count = 8;

	return 0;

error:
	ret = -errno;
	close(dev->fd);
	return ret;
}

int openbci_create(struct med_eeg **edev, struct med_kv *kv)
{
	struct obci_dev *dev = malloc(sizeof(*dev));
	const char *key, *val;
	int ret, i, chan_cnt = 0;

	memset(dev, 0, sizeof(*dev));

	(*edev) = &dev->edev;
	(*edev)->type           = "openbci";
	(*edev)->channel_count  = 0;

	dev->baud_rate = B115200;

	med_for_each_kv(kv, key, val) {
		med_dbg(*edev, "Parsing %s=%s", key, val);

		if (!strcmp("port", key))
			dev->port = strdup(val);
	}

	ret = open_port(dev);

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
