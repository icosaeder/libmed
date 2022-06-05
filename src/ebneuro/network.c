// SPDX-License-Identifier: GPL-3.0-only

/*
 * network.c - messaging helpers.
 */

#include <stdint.h>
#include <string.h>

#include "../system/system.h"
#include "../system/endiannes.h"
#include "../system/helpers.h"

#include "packets.h"
#include "ebneuro_private.h"

/**
 * eb_send() - Send the request with payload.
 * @fd:		Socket fd.
 * @pid:	Packet ID.
 * @buf:	Payload.
 * @len:	Payload length.
 *
 * Return: As s_send.
 */
int eb_send(int fd, uint8_t pid, const void *buf, uint16_t len)
{
	int buflen = sizeof(struct eb_packet_hdr) + len + sizeof(uint8_t);
	struct eb_packet_hdr *packet = malloc(buflen);
	int ret;

	eb_dbg("pkt=%d, len=%d", pid, buflen);

	if (!packet) {
		eb_err("Buffer allocation failed. OOM?");
		return -12; // FIXME
	}

	packet->magic = EB_PACKET_START_MAGIC;
	packet->id = pid;
	packet->length = cpu_to_be16(len);
	if (buf)
		memcpy(packet->data, buf, len);
	packet->data[len] = EB_PACKET_END_MAGIC;

	ret = s_send(fd, packet, buflen, 0);
	if (ret < 0)
		eb_err("Packet send failure: %d", ret);

	free(packet);
	return ret;
}

/**
 * eb_send_id() - send a request with no payload.
 */
int eb_send_id(int fd, uint8_t pid)
{
	return eb_send(fd, pid, NULL, 0);
}

/* FIXME: recv's don't expose packet id, but do we care? */

/* FIXME: Does the EEG data packet have the error code?
 * Maybe the err would read some garbae if not, can ignore that.
 *
 * Answer: It did not... And it was a bit quirky, the last field
 * of the data packet will end up in the error code. It's still
 * exposed to the user so this interface still works even though
 * inconvenient. See eb_get_data().
 */

int eb_recv(int fd, void *buf, uint16_t len, int *err)
{
	int buflen = sizeof(struct eb_packet_hdr) +
		     len + sizeof(__le16) + sizeof(uint8_t);
	struct eb_packet_hdr *packet = malloc(buflen);
	int ret;

	if (!packet) {
		eb_err("Buffer allocation failed. OOM?");
		return -12; // FIXME
	}

	ret = s_recv(fd, packet, buflen, MSG_WAITALL);
	if (ret < 0) {
		eb_err("Packet recv failure: %d", ret);
		goto error;
	}

	if (packet->magic != EB_PACKET_START_MAGIC) {
		eb_err("Packet start magic is incorrect.");
		goto error;
	}

	// TODO: Check end magic as well?

	buflen = be16_to_cpu(packet->length) - sizeof(__le16);
	if (buflen && buf)
		memcpy(buf, packet->data, buflen);
	if (err)
		*err = le16_to_cpu((__le16)(packet->data[buflen]));

error:
	free(packet);
	return ret;
}

/**
 * eb_recv_err() - Receive a response with no payload expected.
 */
int eb_recv_err(int fd)
{
	int ret, err;

	ret = eb_recv(fd, NULL, 0, &err);
	if (ret < 0)
		return ret;

	return err;
}

/**
 * eb_send_recv_err() - Send message and expect empty return.
 */
int eb_send_recv_err(int fd, uint8_t pid, const void *buf, uint16_t len)
{
	int ret = eb_send(fd, pid, buf, len);
	if (ret < 0)
		return ret;

	return eb_recv_err(fd);
}

/**
 * eb_request_info() - Send ID and get the payload.
 */
int eb_request_info(int fd, uint8_t pid, void *buf, uint16_t len)
{
	int err, ret = eb_send_id(fd, pid);
	if (ret < 0)
		return ret;

	ret = eb_recv(fd, buf, len, &err);
	if (ret < 0)
		return ret;

	return err;
}

