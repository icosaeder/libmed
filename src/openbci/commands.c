// SPDX-License-Identifier: GPL-3.0-only

/*
 * commands.c - Implementations of various commands to the hardware.
 */

#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include <system/helpers.h>

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

/**
 * obci_enable_channel() - Enable the channel given.
 */
int obci_enable_channel(struct obci_dev *dev, int chan)
{
	return obci_text_cmd(dev, OPENBCI_CHANNEL_ON(chan), NULL, 0);
}

/**
 * obci_disable_channel() - Disable the channel given.
 */
int obci_disable_channel(struct obci_dev *dev, int chan)
{
	return obci_text_cmd(dev, OPENBCI_CHANNEL_OFF(chan), NULL, 0);
}

/**
 * obci_enable_test_signal() - Set test signal mode.
 * @mode: the command to use to enable the test mode.
 *
 * Use OPENBCI_TEST_SIGNAL_CONNECT_TO_... to set the mode.
 */
int obci_enable_test_signal(struct obci_dev *dev, char mode)
{
	char tmp[64] = {0};

	if (!dev->is_streaming)
		return obci_text_cmd(dev, mode, tmp, sizeof(tmp));
	else
		return obci_text_cmd(dev, mode, NULL, 0);
}

/**
 * obci_set_channel_config() - Set configuration for a single input.
 */
int obci_set_channel_config(struct obci_dev *dev, int chan, bool powerdown,
		int gain, char input_type, bool bias, bool srb2, bool srb1)
{
	char tmp[64] = {0};
	char cmds[] = {
		OPENBCI_CHANNEL_CMD_SET,
		OPENBCI_CHANNEL_CMD_CHANNEL(chan),
		powerdown ? OPENBCI_CHANNEL_CMD_POWER_OFF : OPENBCI_CHANNEL_CMD_POWER_ON,
		OPENBCI_CHANNEL_CMD_GAIN(gain),
		input_type,
		bias ? OPENBCI_CHANNEL_CMD_BIAS_INCLUDE : OPENBCI_CHANNEL_CMD_BIAS_REMOVE,
		srb2 ? OPENBCI_CHANNEL_CMD_SRB2_CONNECT : OPENBCI_CHANNEL_CMD_SRB2_DISCONNECT,
		srb1 ? OPENBCI_CHANNEL_CMD_SRB1_CONNECT : OPENBCI_CHANNEL_CMD_SRB1_DISCONNECT,
		OPENBCI_CHANNEL_CMD_LATCH,
		0,
	};

	if (!dev->is_streaming)
		return obci_text_cmds(dev, cmds, tmp, sizeof(tmp));
	else
		return obci_text_cmds(dev, cmds, NULL, 0);
}

/**
 * obci_restore_defaults() - Set all channels to the default settings.
 */
int obci_restore_defaults(struct obci_dev *dev)
{
	char tmp[64] = {0};
	char cmd = OPENBCI_CHANNEL_DEFAULT_ALL_SET;

	if (!dev->is_streaming)
		return obci_text_cmd(dev, cmd, tmp, sizeof(tmp));
	else
		return obci_text_cmd(dev, cmd, NULL, 0);
}

/**
 * obci_set_leadoff_impedance() - Set impeadence test signal on a given channel.
 */
int obci_set_leadoff_impedance(struct obci_dev *dev, int chan, bool pchan, bool nchan)
{
	char tmp[64] = {0};
	char cmds[] = {
		OPENBCI_CHANNEL_IMPEDANCE_SET,
		OPENBCI_CHANNEL_CMD_CHANNEL(chan),
		pchan ? OPENBCI_CHANNEL_IMPEDANCE_TEST_SIGNAL_APPLIED :OPENBCI_CHANNEL_IMPEDANCE_TEST_SIGNAL_APPLIED_NOT,
		nchan ? OPENBCI_CHANNEL_IMPEDANCE_TEST_SIGNAL_APPLIED :OPENBCI_CHANNEL_IMPEDANCE_TEST_SIGNAL_APPLIED_NOT,
		OPENBCI_CHANNEL_IMPEDANCE_LATCH,
		0,
	};

	if (!dev->is_streaming)
		return obci_text_cmds(dev, cmds, tmp, sizeof(tmp));
	else
		return obci_text_cmds(dev, cmds, NULL, 0);
}

/**
 * obci_set_leadoff_impedance_all() - Set impeadance test signal on all channels.
 */
int obci_set_leadoff_impedance_all(struct obci_dev *dev, bool pchan, bool nchan)
{
	int ret, i;

	for (i = 0; i < dev->edev.channel_count; ++i) {
		ret = obci_set_leadoff_impedance(dev, i, pchan, nchan);
		if (ret < 0)
			return ret;
	}

	return 0;
}

