// SPDX-License-Identifier: GPL-3.0-only

/*
 * packets.c - message sending/receiving helpers.
 */

#include <assert.h>
#include <string.h>

#include <system/system.h>

#include "openbci.h"
#include "packets.h"
#include "OpenBCI_32bit_Library_Definitions.h"

/**
 * obci_text_cmds() - Send a multi char cmd and read a response.
 */
int obci_text_cmds(struct obci_dev *dev, const char *cmd, char *buf, size_t len)
{
	int ret;
	int i = 0;

	/* Writeout cmd bytes. */
	while (*cmd) {
		ret = s_fdputc(*cmd, dev->fd);
		if (ret < 0)
			return ret;
		cmd++;
	}

	/* Read back the response */
	while (len && (i<3 || strncmp(&(buf[i-3]), "$$$", 3))) {
		ret = s_read(dev->fd, &(buf[i]), 1);
		if (ret < 0)
			return ret;
		i++;
		len--;
	}

	return i;
}

/**
 * obci_text_cmd() - Send a single char cmd and read the response.
 */
int obci_text_cmd(struct obci_dev *dev, char cmd, char *buf, size_t len)
{
	char cmds[2] = {cmd, 0};

	return obci_text_cmds(dev, cmds, buf, len);
}

/**
 * obci_read_data_pkt() - Read and sanity-check a data packet.
 */
int obci_read_data_pkt(struct obci_dev *dev, struct openbci_data *data)
{
	int ret;

	assert(sizeof(*data) == OPENBCI_PACKET_SIZE);

	ret = s_read(dev->fd, data, sizeof(*data));
	if (ret < 0)
		return ret;

	s_ddump_data(SPEW, "pkt", data, sizeof(*data));

	assert(ret == OPENBCI_PACKET_SIZE);
	assert(data->magic == OPENBCI_DATA_MAGIC);
	assert((data->stop & 0xf0) == OPENBCI_DATA_END_MAGIC);

	return ret;
}
