// SPDX-License-Identifier: GPL-3.0-only

/*
 * ebneuro.c - various routines for the device.
 */

#include <stdint.h>
#include <string.h>

#include "../system/endiannes.h"
#include "../system/helpers.h"

#include "packets.h"
#include "ebneuro_private.h"

static void eb_add_sample(struct eb_dev *dev, int seq, float *data)
{
	int i = 0;
	struct eb_sample_list *new = malloc(sizeof(*new));

	if (!new) {
		eb_err("Node allocation failed! OOM?");
		return;
	}

	new->next = NULL;
	new->seq = seq;

	for (i = 0; i < EB_BEPLUSLTM_EEG_CHAN; ++i)
		new->eeg[i] = data[i];

	for (i = 0; i < EB_BEPLUSLTM_DC_CHAN; ++i)
		new->dc[i] = data[EB_BEPLUSLTM_EEG_CHAN + i];

	if (!dev->samples) {
		dev->samples = dev->samples_end = new;
	} else {
		dev->samples_end->next = new;
		dev->samples_end = new;
	}
	dev->sample_count++;
}

static void eb_get_sample(struct eb_dev *dev, float *eeg, float *dc)
{
	struct eb_sample_list *old = dev->samples;

	if (!old) {
		eb_err("Sample queue is empty!");
		return;
	}

	memcpy(eeg, old->eeg, EB_BEPLUSLTM_EEG_CHAN * sizeof(*eeg));
	memcpy(dc, old->dc, EB_BEPLUSLTM_DC_CHAN * sizeof(*dc));

	dev->samples = old->next;
	dev->sample_count--;
	free(old);
}

static void eb_delete_data_queue(struct eb_dev *dev)
{
	struct eb_sample_list *next = dev->samples;
	struct eb_sample_list *tmp;

	while (next) {
		tmp = next->next;
		free(next);
		next = tmp;
	}

	dev->samples = NULL;
	dev->samples_end = NULL;
	dev->sample_count = 0;
}

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
int eb_set_mode(struct eb_dev *dev, int mode)
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

	eb_delete_data_queue(dev);
	
	msg.mode = cpu_to_le16(mode);
	return eb_send_recv_err(dev->fd_ctrl, EB_CPK_ID_MODE_SET,
				&msg, sizeof(msg));
}

/**
 * eb_prepare() - Initialize the deivice.
 */
int eb_prepare(struct eb_dev *dev)
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
 * eb_set_default_preset() - Upload a simple preset to the device.
 *
 * This method uses rates set in the object struct.
 */
int eb_set_default_preset(struct eb_dev *dev)
{
	int i, err;
	struct eb_preset data = {
		.name = "default",
		.flags = cpu_to_le16(EB_FLAG_OHM_SIGNAL | EB_FLAG_STIM_MONITOR),
		.mains_rate = cpu_to_le16(50), // FIXME detect?
		.packet_rate = cpu_to_le16(dev->packet_rate),
	};

	for (i = 0; i < EB_BEPLUSLTM_EEG_CHAN; ++i)
		data.eeg_rates[i] = cpu_to_le16(dev->data_rate);

	for (i = 0; i < EB_BEPLUSLTM_DC_CHAN; ++i)
		data.dc_rates[i] = cpu_to_le16(dev->data_rate);
	
	err = eb_send_recv_err(dev->fd_ctrl, EB_CPK_ID_PRESET_UPL,
			       &data, sizeof(data));
	if (err) {
		eb_err("Failed to upload preset: %d", err);
		return err;
	}

	return 0;
}

/**
 * eb_get_data() - Fetch values from the device.
 * @sample_count: Amount of records (not bytes or floats!) to write.
 */
int eb_get_data(struct eb_dev *dev, float *eeg_buf, float *dc_buf, int sample_cnt)
{
	int data_cnt = (dev->data_rate / dev->packet_rate);
	uint32_t seq;
	int i, j, ret = 0;
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
		+ data_cnt * (EB_BEPLUSLTM_EEG_CHAN + EB_BEPLUSLTM_DC_CHAN + 1) * sizeof(__le16)
		+ 2 * sizeof(__le16);
	float converted[EB_BEPLUSLTM_EEG_CHAN + EB_BEPLUSLTM_DC_CHAN] = {0};

	__le16 *data = malloc(buflen);
	if (!data) {
		eb_err("Buffer allocation failed! OOM?");
		return -12;
	}

	while (dev->sample_count < sample_cnt) {
		/*
		 * XXX HACK: eb_recv expects the device to append the __le16
		 * error code to the message so we shall decrease the length
		 * by that value. This means that the "pulse_duration" value
		 * will be read into the error code.
		 * We ignore it here for now anyway.
		 */
		ret = eb_recv(dev->fd_data, data, buflen - sizeof(__le16), NULL);
		if (ret < 0) {
			eb_err("Data recieval failure: %d", ret);
			goto error;
		}

		seq = le32_to_cpu(*(__le32*)(data));
		
		for (i = 0; i < data_cnt; ++i) {
			for (j = 0; j < EB_BEPLUSLTM_EEG_CHAN; ++j)
				converted[j] = 0.125f * le16_to_cpu(
						data[2 + i*(EB_BEPLUSLTM_EEG_CHAN) + j]
						);

			// TODO: the device has multiple modes.
			for (j = 0; j < EB_BEPLUSLTM_DC_CHAN; ++j)
				converted[EB_BEPLUSLTM_EEG_CHAN+j] = 15.25f * le16_to_cpu(
						data[2 + data_cnt*EB_BEPLUSLTM_EEG_CHAN
							+ i*(EB_BEPLUSLTM_DC_CHAN) + j]
						);
			
			eb_add_sample(dev, seq, converted);
		} 
	}

	for (i = 0; i < sample_cnt; ++i) {
		eb_get_sample(dev, &eeg_buf[EB_BEPLUSLTM_EEG_CHAN*i],
				&dc_buf[EB_BEPLUSLTM_DC_CHAN*i]);
	}

error:
	free(data);
	return ret;
}

/**
 * eb_get_impedances() - read impedance data from the device.
 */
int eb_get_impedances(struct eb_dev *dev, short *eeg, short *dc)
{
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
		eeg[i] =  (int16_t)le16_to_cpu(data.eeg[i].p)
			+ (int16_t)le16_to_cpu(data.eeg[i].n);

	for (i = 0; i < EB_BEPLUSLTM_DC_CHAN; ++i)
		dc[i] =   (int16_t)le16_to_cpu(data.dc[i].p)
			+ (int16_t)le16_to_cpu(data.dc[i].n);

	return 0;
}

/**
 * eb_unprepare() - De-initialize the deivice.
 */
int eb_unprepare(struct eb_dev *dev)
{
	int err;

	err = eb_set_mode(dev, EB_MODE_IDLE);
	if (err) {
		eb_err("Failed to set idle mode: %d", err);
		return err;
	}

	s_close(dev->fd_ctrl);
	s_close(dev->fd_data);

	err = s_connect(&dev->fd_init, dev->ipaddr, EB_SOCK_PORT_INIT);
	if (err)
		return err;

	err = eb_set_socket_state(dev, EB_SOCK_INDEX_CTRL, EB_SOCK_STATE_DISABLE);
	if (err) {
		eb_err("Failed to disable control socket: %d", err);
		return err;
	}

	err = eb_set_socket_state(dev, EB_SOCK_INDEX_DATA, EB_SOCK_STATE_DISABLE);
	if (err) {
		eb_err("Failed to disable data socket: %d", err);
		return err;
	}

	s_close(dev->fd_init);

	eb_delete_data_queue(dev);

	return 0;
}

