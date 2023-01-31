// SPDX-License-Identifier: GPL-3.0-only

/*
 * ebneuro.c - various routines for the device.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <system/endiannes.h>
#include <system/helpers.h>

#include "packets.h"
#include "ebneuro.h"

/**
 * eb_set_socket_state() - Set the state of the remote socket.
 */
static int eb_set_socket_state(struct eb_dev *dev, int index, int state)
{
	struct eb_sock_state msg = {
		.index = cpu_to_le16(index),
		.state = cpu_to_le16(state),
	};

	return eb_send_recv_err(dev->fd_init, EB_IPK_ID_SET_SOCK,
				&msg, sizeof(msg));
}

/**
 * eb_set_mode() - Set the device operation mode.
 *
 * This method will also flush the pending data and internal queues.
 */
static int eb_set_mode(struct eb_dev *dev, int mode)
{
	int err;
	struct eb_mode msg = {
		.mode = cpu_to_le16(EB_MODE_IDLE),
	};

	err = eb_send_recv_err(dev->fd_ctrl, EB_CPK_ID_MODE_SET,
				&msg, sizeof(msg));
	if (err) {
		eb_err("Failed to set idle mode: %d", err);
		return err;
	}

	err = s_flush(dev->fd_data);
	if (err < 0) {
		eb_err("Failed to flush the data socket: %d", err);
		return err;
	}
	eb_dbg("Flushed %d pending bytes.", err);

	msg.mode = cpu_to_le16(mode);
	return eb_send_recv_err(dev->fd_ctrl, EB_CPK_ID_MODE_SET,
				&msg, sizeof(msg));
}

/**
 * eb_prepare() - Initialize the deivice.
 */
static int eb_prepare(struct eb_dev *dev)
{
	int err;
	struct eb_client_set cl_msg = { 0 }; // FIXME

	eb_dbg("Preparing the connection to %s ...", dev->ipaddr);

	err = s_connect(&dev->fd_init, dev->ipaddr, EB_SOCK_PORT_INIT);
	if (err < 0) {
		eb_err("Device connection failed: %d", err);
		return err;
	}

	/* Exchange client data. */
	err = eb_request_info(dev->fd_init, EB_IPK_ID_CLIENT,
			      &dev->client, sizeof(dev->client));
	if (err) {
		eb_err("Info request failed: %d", err);
		return err;
	}

	err = eb_send_recv_err(dev->fd_init, EB_IPK_ID_CLIENT_SET,
			       &cl_msg, sizeof(cl_msg));
	if (err) {
		eb_err("Uploading client info failed: %d", err);
		return err;
	}

	/* Survey hardware info. */
	err = eb_request_info(dev->fd_init, EB_IPK_ID_DEVICE,
			      &dev->dev_info, sizeof(dev->dev_info));
	if (err) {
		eb_err("Firmware info request failed: %d", err);
		return err;
	}
	eb_info("Connected to \"%s\"", (char*)dev->dev_info.name);

	err = eb_request_info(dev->fd_init, EB_IPK_ID_FIRMWARE,
			      &dev->fw_info, sizeof(dev->fw_info));
	if (err) {
		eb_err("Firmware info request failed: %d", err);
		return err;
	}
	
	err = eb_request_info(dev->fd_init, EB_IPK_ID_HARDWARE,
			      &dev->hw_info, sizeof(dev->hw_info));
	if (err) {
		eb_err("Hardware info request failed: %d", err);
		return err;
	}

	/* Open sockets. */
	err = eb_set_socket_state(dev, EB_SOCK_INDEX_CTRL, EB_SOCK_STATE_ENABLE);
	if (err) {
		eb_err("Failed to enable control socket: %d", err);
		return err;
	}

	err = eb_set_socket_state(dev, EB_SOCK_INDEX_CTRL, EB_SOCK_STATE_START);
	if (err) {
		eb_err("Failed to start control socket: %d", err);
		return err;
	}

	err = eb_set_socket_state(dev, EB_SOCK_INDEX_DATA, EB_SOCK_STATE_ENABLE);
	if (err) {
		eb_err("Failed to enable data socket: %d", err);
		return err;
	}

	err = eb_set_socket_state(dev, EB_SOCK_INDEX_DATA, EB_SOCK_STATE_START);
	if (err) {
		eb_err("Failed to start data socket: %d", err);
		return err;
	}

	s_close(dev->fd_init);

	err = s_connect(&dev->fd_ctrl, dev->ipaddr, EB_SOCK_PORT_CTRL);
	if (err) {
		eb_err("Failed to connect to the control socket: %d", err);
		return err;
	}

	err = s_connect(&dev->fd_data, dev->ipaddr, EB_SOCK_PORT_DATA);
	if (err) {
		eb_err("Failed to connect to the data socket: %d", err);
		return err;
	}

	err = eb_set_mode(dev, EB_MODE_IDLE);
	if (err) {
		eb_err("Failed to set idle mode: %d", err);
		return err;
	}

	return 0;
}

/**
 * eb_set_preset() - Upload a simple preset to the device.
 * @packet_rate:	Amount of packets to be sent per second.
 * @data_rate:		Amount of records to be sent per second.
 * 
 * Amount of records per packet will be data_rate / packet_rate.
 */
static int eb_set_preset(struct eb_dev *dev, int packet_rate, int data_rate)
{
	int i, err;
	struct eb_preset data = {
		.name = "default",
		.flags = cpu_to_le16(EB_FLAG_OHM_SIGNAL | EB_FLAG_STIM_MONITOR),
		.mains_rate = cpu_to_le16(50), // FIXME detect?
		.packet_rate = cpu_to_le16(packet_rate),
	};

	for (i = 0; i < EB_BEPLUSLTM_EEG_CHAN; ++i)
		data.eeg_rates[i] = cpu_to_le16(data_rate);

	for (i = 0; i < EB_BEPLUSLTM_DC_CHAN; ++i)
		data.dc_rates[i] = cpu_to_le16(data_rate);
	
	err = eb_send_recv_err(dev->fd_ctrl, EB_CPK_ID_PRESET_UPL,
			       &data, sizeof(data));
	if (err) {
		eb_err("Failed to upload preset: %d", err);
		return err;
	}

	dev->data_rate = data_rate;
	dev->packet_rate = packet_rate;

	return 0;
}

static int ebneuro_sample(struct med_eeg *edev)
{
	struct eb_dev *dev = container_of(edev, struct eb_dev, edev);
	int sample_cnt = (dev->data_rate / dev->packet_rate);
	struct med_sample *next;
	int i, j, ret = 0;
	uint32_t seq;
	/*
	 * Packet format:
	 *	le32 seq;
	 *	le16 eeg[EEG_CHAN][cnt];
	 *	le16 dc[DC_CHAN][cnt];
	 *	le16 svc[1][cnt];
	 *	be16 pulse_rate;
	 *	be16 pulse_duration;
	 */
	int buflen = sizeof(__le32) 
		+ sample_cnt * (EB_BEPLUSLTM_EEG_CHAN + EB_BEPLUSLTM_DC_CHAN + 1) * sizeof(__le16)
		+ 2 * sizeof(__le16);

	__le16 *data, *buffer = malloc(buflen);

	/*
	 * XXX HACK: eb_recv expects the device to append the __le16
	 * error code to the message so we shall decrease the length
	 * by that value. This means that the "pulse_duration" value
	 * will be read into the error code.
	 * We ignore it here for now anyway.
	 */
	ret = eb_recv(dev->fd_data, buffer, buflen - sizeof(__be16), NULL);
	if (ret < 0) {
		eb_err("Data recieval failure: %d", ret);
		goto error;
	}

	seq = le32_to_cpu(*(__le32*)(buffer));
	data = buffer + 2;
	
	for (i = 0; i < sample_cnt; ++i) {
		next = med_eeg_alloc_sample(edev);

		for (j = 0; j < EB_BEPLUSLTM_EEG_CHAN; ++j)
			next->data[j] = 0.125f * le16_to_cpu(
					data[i*(EB_BEPLUSLTM_EEG_CHAN) + j]
					);

		// TODO: the device has multiple modes.
		for (j = 0; j < EB_BEPLUSLTM_DC_CHAN; ++j)
			next->data[EB_BEPLUSLTM_EEG_CHAN+j] = 15.25f * le16_to_cpu(
					data[sample_cnt*EB_BEPLUSLTM_EEG_CHAN
						+ i*(EB_BEPLUSLTM_DC_CHAN) + j]
					);

		med_eeg_add_sample(edev, next);
	} 

	ret = sample_cnt;

error:
	free(buffer);
	return ret;
}

static int ebneuro_get_impedance(struct med_eeg *edev, float *samples)
{
	struct eb_dev *dev = container_of(edev, struct eb_dev, edev);
	struct eb_impedance_info data;
	int err, i;

	err = eb_request_info(dev->fd_init, EB_CPK_ID_IMPEDANCE,
			      &data, sizeof(data));
	if (err) {
		eb_err("Impedance info request failed: %d", err);
		return err;
	}

	/* The devcie sends dubious packets on the DATA channel
	 * that we must destroy.
	 */
	err = s_flush(dev->fd_data);
	if (err < 0) {
		eb_err("Failed to flush the data socket: %d", err);
		return err;
	}
	eb_dbg("Flushed %d pending bytes.", err);

	for (i = 0; i < EB_BEPLUSLTM_EEG_CHAN; ++i)
		samples[i] = (int16_t)le16_to_cpu(data.eeg[i].p)
			   + (int16_t)le16_to_cpu(data.eeg[i].n);

	/* The hardware seems to send dubious data on DC channels. Ignore. */
	for (; i < EB_BEPLUSLTM_EEG_CHAN + EB_BEPLUSLTM_DC_CHAN; ++i)
		samples[i] = -1.;

	return EB_BEPLUSLTM_EEG_CHAN + EB_BEPLUSLTM_DC_CHAN;
}

static int ebneuro_set_mode(struct med_eeg *edev, enum med_eeg_mode mode)
{
	struct eb_dev *dev = container_of(edev, struct eb_dev, edev);

	switch (mode) {
	case MED_EEG_IDLE:
		return eb_set_mode(dev, EB_MODE_IDLE);
	case MED_EEG_SAMPLING:
		return eb_set_mode(dev, EB_MODE_SAMPLE);
	case MED_EEG_IMPEDANCE:
		return eb_set_mode(dev, EB_MODE_IMPEDANCE);
	case MED_EEG_TEST:
		return eb_set_mode(dev, EB_MODE_WAVE);
	}
}

static void ebneuro_destroy(struct med_eeg *edev)
{
	struct eb_dev *dev = container_of(edev, struct eb_dev, edev);
	int i, err;

	err = eb_set_mode(dev, EB_MODE_IDLE);
	if (err)
		eb_err("Failed to set idle mode: %d", err);

	s_close(dev->fd_ctrl);
	s_close(dev->fd_data);

	err = s_connect(&dev->fd_init, dev->ipaddr, EB_SOCK_PORT_INIT);
	if (err) {
		eb_err("Failed to connect to init socket: %d", err);
		return;
	}

	err = eb_set_socket_state(dev, EB_SOCK_INDEX_CTRL, EB_SOCK_STATE_DISABLE);
	if (err)
		eb_err("Failed to disable control socket: %d", err);

	err = eb_set_socket_state(dev, EB_SOCK_INDEX_DATA, EB_SOCK_STATE_DISABLE);
	if (err)
		eb_err("Failed to disable data socket: %d", err);

	s_close(dev->fd_init);

	for (i = 0; i < edev->channel_count; ++i)
		free(edev->channel_labels[i]);

	free(edev->channel_labels);
	free(dev);
}

int ebneuro_create(struct med_eeg **edev, struct med_kv *kv)
{
	int ret, i, chan_cnt = EB_BEPLUSLTM_EEG_CHAN + EB_BEPLUSLTM_DC_CHAN;
	struct eb_dev *dev = malloc(sizeof(*dev));
	int packet_rate = 64, data_rate = 512;
	char *key, *val;

	memset(dev, 0, sizeof(*dev));

	(*edev) = &dev->edev;
	(*edev)->type          = "ebneuro";

	med_for_each_kv(kv, key, val) {
		med_dbg(*edev, "Parsing %s=%s", key, val);

		if (!strcmp("address", key))
			strncpy(dev->ipaddr, val, sizeof(dev->ipaddr));
		if (!strcmp("packet_rate", key))
			packet_rate = atoi(val);
		if (!strcmp("data_rate", key))
			data_rate = atoi(val);
	}

	(*edev)->channel_count  = chan_cnt;
	(*edev)->channel_labels = malloc(sizeof(char**) * chan_cnt);

	for (i = 0; i < EB_BEPLUSLTM_EEG_CHAN; ++i) {
		(*edev)->channel_labels[i] = malloc(sizeof(char) * 8);
		snprintf((*edev)->channel_labels[i], 8, "eeg%d", i);
	}

	for (i = 0; i < EB_BEPLUSLTM_DC_CHAN; ++i) {
		(*edev)->channel_labels[EB_BEPLUSLTM_EEG_CHAN + i] = malloc(sizeof(char) * 8);
		snprintf((*edev)->channel_labels[EB_BEPLUSLTM_EEG_CHAN + i], 8, "dc%d", i);
	}

	(*edev)->sample         = ebneuro_sample;
	(*edev)->get_impedance  = ebneuro_get_impedance;
	(*edev)->set_mode       = ebneuro_set_mode;
	(*edev)->destroy        = ebneuro_destroy;

	ret = eb_prepare(dev);
	if (ret)
		goto error;

	ret = eb_set_preset(dev, packet_rate, data_rate);
	if (ret)
		goto error;


	return 0;

error:
	free(dev);
	(*edev) = NULL;
	return ret;
}
