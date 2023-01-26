/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef EBNEURO_H
#define EBNEURO_H

#include <stdint.h>

#include <system/system.h>
#include <system/endiannes.h>
#include <system/helpers.h>

#include <med/eeg_priv.h>

#include "packets.h"

/**
 * struct eb_dev - ebneuro device.
 * @ipaddr:		IP of the device.
 * @packet_rate:	Desired amount of packets per second
 * @data_rate:		Desired amount of samples per second.
 */
struct eb_dev {

	struct med_eeg edev;

	char ipaddr[17];

	int fd_init;
	int fd_ctrl;
	int fd_data;

	struct eb_device dev_info;
	struct eb_client client;
	struct eb_firmware fw_info;
	struct eb_hardware hw_info;

	int packet_rate;
	int data_rate;
};

/* network.c */
int eb_send(int fd, uint8_t pid, const void *buf, uint16_t len);
int eb_send_id(int fd, uint8_t pid);
int eb_recv(int fd, void *buf, uint16_t len, int *err);
int eb_recv_err(int fd);
int eb_send_recv_err(int fd, uint8_t pid, const void *buf, uint16_t len);
int eb_request_info(int fd, uint8_t pid, void *buf, uint16_t len);

/* debug print helpers */
#define eb_err(fmt, ...) \
	s_dprintf(CRITICAL, "[ebneuro] %s:%d: " fmt "\n", \
			__func__, __LINE__, ##__VA_ARGS__)

#define eb_info(fmt, ...) \
	s_dprintf(INFO, "[ebneuro] %s:%d: " fmt "\n", \
			__func__, __LINE__, ##__VA_ARGS__)

#define eb_dbg(fmt, ...) \
	s_dprintf(SPEW, "[ebneuro] %s:%d: " fmt "\n", \
			__func__, __LINE__, ##__VA_ARGS__)

#endif /* EBNEURO_H */
