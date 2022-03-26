
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
	struct eb_mode msg {
		.mode = cpu_to_le16(mode),
	}

	return eb_send_recv_err(dev->fd_ctrl,
				EB_CPK_ID_MODE_SET,
				&msg, sizeof(msg));
}

/**
 * eb_prepare() - Initialize the deivice.
 */
int eb_prepare(struct eb_dev *dev)
{
	int ret;
	struct eb_client_set cl_msg = { 0 }; // FIXME

	ret = s_connect(&dev->fd_init, dev->ipaddr, 50001); // FIXME
	if (ret < 0)
		return ret;

	/* Exchange client data. */
	ret = eb_request_info(dev->fd_init,
			      EB_IPK_ID_CLIENT,
			      &dev->client, sizeof(dev->client));
	if (ret != 0) {
		s_dprintf(CRITICAL, "%s:%d: unexpected: %d\n",
				__func__, __LINE__, ret);
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
			      &dev->fw_info, sizeof(dev->fw_info));
	if (ret != 0) {
		s_dprintf(CRITICAL, "%s:%d: unexpected: %d\n",
				__func__, __LINE__, ret);
		return ret;
	}
	
	ret = eb_request_info(dev->fd_init,
			      EB_IPK_ID_HARDWARE,
			      &dev->hw_info, sizeof(dev->hw_info));
	if (ret != 0) {
		s_dprintf(CRITICAL, "%s:%d: unexpected: %d\n",
				__func__, __LINE__, ret);
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

	ret = s_connect(&dev->fd_ctrl, dev->ipaddr,
			le16_to_cpu(dev->client.control_sock.port));
	if (ret < 0)
		return ret;

	ret = s_connect(&dev->fd_data, dev->ipaddr,
			le16_to_cpu(dev->client.data_sock.port));
	if (ret < 0)
		return ret;

	return 0;
}

/**
 * eb_prepare() - De-initialize the deivice.
 */
int eb_unprepare(struct eb_dev *dev)
{
	int ret;

	s_close(dev->fd_ctrl);
	s_close(dev->fd_data);

	ret = s_connect(&dev->fd_init, dev->ipaddr, 50001); // FIXME
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

	return 0;
}

