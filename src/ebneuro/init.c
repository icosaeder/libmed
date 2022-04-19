
/*
 * init.c - various initialization routines for the device.
 */

#include <stdint.h>

#include "../system/endiannes.h"
#include "../system/helpers.h"

#include "packets.h"
#include "ebneuro.h"

/**
 * eb_set_socket_state() - Set the state of the remote socket.
 */
int eb_set_socket_state(struct eb_dev *dev, int index, int state)
{
	struct eb_sock_state msg = {
		.index = cpu_to_le16(index),
		.state = cpu_to_le16(state),
	};

	return eb_send_recv_err(dev->fd_init,
				EB_IPK_ID_SET_SOCK,
				&msg, sizeof(msg));
}

/**
 * eb_set_mode() - Set the device operation mode.
 */
int eb_set_mode(struct eb_dev *dev, int mode)
{
	struct eb_mode msg = {
		.mode = cpu_to_le16(mode),
	};

	return eb_send_recv_err(dev->fd_ctrl,
				EB_CPK_ID_MODE_SET,
				&msg, sizeof(msg));
}

/**
 * eb_prepare() - Initialize the deivice.
 */
int eb_prepare(struct eb_dev *dev)
{
	int ret, err;
	struct eb_client_set cl_msg = { 0 }; // FIXME

	ret = s_connect(&dev->fd_init, dev->ipaddr, EB_SOCK_PORT_INIT);
	if (ret < 0)
		return ret;

	/* Exchange client data. */
	ret = eb_request_info(dev->fd_init,
			      EB_IPK_ID_CLIENT,
			      &dev->client, sizeof(dev->client), &err);
	if (err != 0) {
		s_dprintf(CRITICAL, "%s:%d: unexpected: %d\n",
				__func__, __LINE__, err);
		return ret;
	}

	s_dprintf(INFO, "ebneuro: name=%s\n", (char*)dev->client.name);

	ret = eb_send_recv_err(dev->fd_init,
			       EB_IPK_ID_CLIENT_SET,
			       &cl_msg, sizeof(cl_msg));
	if (ret != 0) {
		s_dprintf(CRITICAL, "%s:%d: unexpected: %d\n",
				__func__, __LINE__, ret);
		return ret;
	}

	/* Survey hardware info. */
	ret = eb_request_info(dev->fd_init,
			      EB_IPK_ID_FIRMWARE,
			      &dev->fw_info, sizeof(dev->fw_info), &err);
	if (err != 0) {
		s_dprintf(CRITICAL, "%s:%d: unexpected: %d\n",
				__func__, __LINE__, err);
		return ret;
	}
	
	ret = eb_request_info(dev->fd_init,
			      EB_IPK_ID_HARDWARE,
			      &dev->hw_info, sizeof(dev->hw_info), &err);
	if (err != 0) {
		s_dprintf(CRITICAL, "%s:%d: unexpected: %d\n",
				__func__, __LINE__, err);
		return ret;
	}

	/* Open sockets. */

	ret = eb_set_socket_state(dev, EB_SOCK_INDEX_CTRL, EB_SOCK_STATE_ENABLE);
	if (ret != 0) {
		s_dprintf(CRITICAL, "%s:%d: unexpected: %d\n",
				__func__, __LINE__, ret);
		return ret;
	}

	ret = eb_set_socket_state(dev, EB_SOCK_INDEX_CTRL, EB_SOCK_STATE_START);
	if (ret != 0) {
		s_dprintf(CRITICAL, "%s:%d: unexpected: %d\n",
				__func__, __LINE__, ret);
		return ret;
	}

	ret = eb_set_socket_state(dev, EB_SOCK_INDEX_DATA, EB_SOCK_STATE_ENABLE);
	if (ret != 0) {
		s_dprintf(CRITICAL, "%s:%d: unexpected: %d\n",
				__func__, __LINE__, ret);
		return ret;
	}

	ret = eb_set_socket_state(dev, EB_SOCK_INDEX_DATA, EB_SOCK_STATE_START);
	if (ret != 0) {
		s_dprintf(CRITICAL, "%s:%d: unexpected: %d\n",
				__func__, __LINE__, ret);
		return ret;
	}

	s_close(dev->fd_init);

	ret = s_connect(&dev->fd_ctrl, dev->ipaddr, EB_SOCK_PORT_CTRL);
	if (ret < 0)
		return ret;

	ret = s_connect(&dev->fd_data, dev->ipaddr, EB_SOCK_PORT_DATA);
	if (ret < 0)
		return ret;

	return 0;
}

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
	
	err = eb_send_recv_err(dev->fd_ctrl,
			       EB_CPK_ID_PRESET_UPL,
			       &data, sizeof(data));
	if (err != 0) {
		s_dprintf(CRITICAL, "%s:%d: unexpected: %d\n",
				__func__, __LINE__, err);
		return err;
	}

	return 0;
}

static void eb_add_sample(struct eb_dev *dev, int seq, float *data)
{
	int i = 0;
	struct eb_sample_list *new = malloc(sizeof(*new));
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
	int i;
	struct eb_sample_list *old = dev->samples;
	if (!old) {
		s_dprintf(CRITICAL, "%s:%d: sample queue is empty\n",
					__func__, __LINE__);
		return;
	}
	dev->samples = old->next;


	// FIXME memcpy
	for (i = 0; i < EB_BEPLUSLTM_EEG_CHAN; ++i)
		eeg[i] = old->eeg[i];

	for (i = 0; i < EB_BEPLUSLTM_DC_CHAN; ++i)
		dc[i] = old->dc[i];

	dev->sample_count--;
	free(old);
}

int eb_get_data(struct eb_dev *dev, float *eeg_buf, float *dc_buf, int sample_cnt)
{
	int data_cnt = (dev->data_rate / dev->packet_rate);
	uint32_t seq;
	int i, j, ret = 0, err, buflen = sizeof(__le32) 
		+ data_cnt * (EB_BEPLUSLTM_EEG_CHAN + EB_BEPLUSLTM_DC_CHAN + 1) * sizeof(__le16)
		+ 2 * sizeof(__le16);
	float converted[EB_BEPLUSLTM_EEG_CHAN + EB_BEPLUSLTM_DC_CHAN] = {0};

	__le16 *data = malloc(buflen);
	if (!data) {
		s_dprintf(CRITICAL, "%s:%d: OOM?\n", __func__, __LINE__);
		return -1;
	}

	while (dev->sample_count < sample_cnt) {
		ret = eb_recv(dev->fd_data, data, buflen - sizeof(__le16), NULL); // XXX dirty hack
		if (ret < 0) {
			s_dprintf(CRITICAL, "%s:%d: unexpected: err=%d, ret=%d\n",
					__func__, __LINE__, err, ret);
			goto error;
		}

		seq = le32_to_cpu(*(__le32*)(data));
		
		for (i = 0; i < data_cnt; ++i) {
			for (j = 0; j < EB_BEPLUSLTM_EEG_CHAN; ++j)
				converted[j] = 0.125f * le16_to_cpu(
						data[2 + i*(EB_BEPLUSLTM_EEG_CHAN) + j]
						);

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
 * eb_prepare() - De-initialize the deivice.
 */
int eb_unprepare(struct eb_dev *dev)
{

	//FIXME delete list
	int ret;

	s_close(dev->fd_ctrl);
	s_close(dev->fd_data);

	ret = s_connect(&dev->fd_init, dev->ipaddr, EB_SOCK_PORT_INIT);
	if (ret < 0)
		return ret;

	ret = eb_set_socket_state(dev, EB_SOCK_INDEX_CTRL, EB_SOCK_STATE_DISABLE);
	if (ret != 0) {
		s_dprintf(CRITICAL, "%s:%d: unexpected: %d\n",
				__func__, __LINE__, ret);
		return ret;
	}

	ret = eb_set_socket_state(dev, EB_SOCK_INDEX_DATA, EB_SOCK_STATE_DISABLE);
	if (ret != 0) {
		s_dprintf(CRITICAL, "%s:%d: unexpected: %d\n",
				__func__, __LINE__, ret);
		return ret;
	}

	// FIXME these are acked but never answered?

	s_close(dev->fd_init);
	return 0;
}

