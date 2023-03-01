// SPDX-License-Identifier: GPL-3.0-only

/*
 * commands.c - Implementations of various commands to the hardware.
 */

#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "openbci.h"
#include "packets.h"
#include "OpenBCI_32bit_Library_Definitions.h"

/**
 * obci_reset() - Reset the board.
 */
int obci_reset(struct obci_dev *dev)
{
	unsigned char buf[400] = {0};
	int ret;

	ret = s_serial_flush(dev->fd);
	if (ret < 0)
		return ret;

	ret = obci_text_cmd(dev, OPENBCI_MISC_SOFT_RESET, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	med_dbg(&dev->edev, "Got version header:\n%s\n", buf);

	return 0;
}

/**
 * obci_get_version() - Read the device version.
 */
int obci_get_version(struct obci_dev *dev)
{
	unsigned char buf[64] = {0};
	int major, minor, patch;
	int ret;

	ret = obci_text_cmd(dev, OPENBCI_GET_VERSION, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	ret = sscanf(buf, "v%d.%d.%d$$$", &major, &minor, &patch);
	if (ret != 3)
		return -EINVAL;

	med_info(&dev->edev, "OpenBCI firmware version: v%d.%d.%d\n", major, minor, patch);
	
	return (major << 16) & (minor << 8) & patch;
}

/**
 * obci_get_max_channels() - Probe for channel count.
 */
int obci_get_max_channels(struct obci_dev *dev)
{
	unsigned char buf[64] = {0};
	int ret;

	ret = obci_text_cmd(dev, OPENBCI_CHANNEL_MAX_NUMBER_16, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	med_dbg(&dev->edev, "buf=%s\n", buf);

	if (strstr(buf, "16$$$"))
		return 16;

	return 8;
}

/**
 * obci_set_max_channels() - set a channel count for the device.
 */
int obci_set_max_channels(struct obci_dev *dev, int channels)
{
	unsigned char buf[64] = {0};
	int ret;

	switch (channels) {
		case 8:
			return obci_text_cmd(dev, OPENBCI_CHANNEL_MAX_NUMBER_8, buf, sizeof(buf));

		case 16:
			ret = obci_text_cmd(dev, OPENBCI_CHANNEL_MAX_NUMBER_16, buf, sizeof(buf));
			if (ret < 0)
				return ret;

			if (strstr(buf, "16$$$"))
				return 0;

			return -EINVAL;

		default:
			return -EINVAL;

	}
}

/**
 * obci_set_streaming() - Switch the streaming state.
 */
int obci_set_streaming(struct obci_dev *dev, bool streaming)
{
	char cmd = (streaming ? OPENBCI_STREAM_START : OPENBCI_STREAM_STOP);
	int ret;

	dev->is_streaming = streaming;

	ret = obci_text_cmd(dev, cmd, NULL, 0);
	if (ret < 0)
		return ret;

	if (!streaming)
		return s_serial_flush(dev->fd);

	return 0;
}

