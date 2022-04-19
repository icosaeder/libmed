
/*
 * network.c - messaging helpers.
 */

#include <stdint.h>
#include <string.h>

#include "../system/system.h"
#include "../system/endiannes.h"
#include "../system/helpers.h"

#include "packets.h"
#include "ebneuro.h"

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
	int buflen = sizeof(struct eb_packet_hdr) +
		     len + sizeof(uint8_t);
	void *packet = malloc(buflen);
	int ret;

	s_dprintf(SPEW, "%s: pkt=%d, len=%d\n", __func__, pid, buflen);

	if (!packet) {
		s_dprintf(CRITICAL,
			  "%s: buffer allocation failed.\n", __func__);
		return -12; // FIXME
	}

	((struct eb_packet_hdr*)packet)->magic = EB_PACKET_START_MAGIC;
	((struct eb_packet_hdr*)packet)->id = pid;
	((struct eb_packet_hdr*)packet)->length = cpu_to_be16(len);
	if (buf)
		memcpy(packet + sizeof(struct eb_packet_hdr), buf, len);
	*(uint8_t*)(packet + sizeof(struct eb_packet_hdr) + len) = EB_PACKET_END_MAGIC;

	ret = s_send(fd, packet, buflen, 0);
	if (ret < 0)
		s_dprintf(CRITICAL,
			  "%s: packet send failure: %d\n", __func__, ret);
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
 * Answer: It did not...
 */

static int eb_recv_flags(int fd, void *buf, uint16_t len, int *err, int flags)
{
	int buflen = sizeof(struct eb_packet_hdr) +
		     len + sizeof(__le16) + sizeof(uint8_t);
	void *packet = malloc(buflen);
	int ret;

	if (!packet) {
		s_dprintf(CRITICAL,
			  "%s: buffer allocation failed.\n", __func__);
		return -12; // FIXME
	}

	ret = s_recv(fd, packet, buflen, flags);
	if (ret < 0) {
		s_dprintf(CRITICAL,
			  "%s: packet recv failure: %d\n", __func__, ret);
		goto error;
	}

	if (*(uint8_t*)packet != EB_PACKET_START_MAGIC) {
		s_dprintf(CRITICAL,
			  "%s: packet has wrong start\n", __func__);
		goto error;
	}

	// TODO: Check end magic as well?

	buflen = be16_to_cpu(((struct eb_packet_hdr*)packet)->length) - sizeof(__le16);
	if (buflen && buf)
		memcpy(buf, packet + sizeof(struct eb_packet_hdr), buflen);
	if (err)
		*err = le16_to_cpu(*(__le16*)(packet+sizeof(struct eb_packet_hdr)
					+ (buflen)));

error:
	free(packet);
	return ret;
}

/**
 * eb_recv() - Receive a response from the device.
 * @err:	Place to save returned error code, can be null.
 */
int eb_recv(int fd, void *buf, uint16_t len, int *err)
{
	return eb_recv_flags(fd, buf, len, err, MSG_WAITALL);
}

/**
 * eb_recv_noblock() - Receive a response from the device.
 * @err:	Place to save returned error code, can be null.
 */
int eb_recv_noblock(int fd, void *buf, uint16_t len, int *err)
{
	return eb_recv_flags(fd, buf, len, err, MSG_DONTWAIT);
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
int eb_request_info(int fd, uint8_t pid, void *buf, uint16_t len, int *err)
{
	int ret = eb_send_id(fd, pid);
	if (ret < 0)
		return ret;

	return eb_recv(fd, buf, len, err);
}

