#ifndef EBNEURO_H
#define EBNEURO_H

#include <stdint.h>

#include "../system/system.h"
#include "../system/endiannes.h"
#include "../system/helpers.h"

#include "packets.h"

/**
 * struct eb_dev - ebneuro device.
 * @ipaddr:		(*) IP of the device.
 * @packet_rate:	(*) Desired amount of packets per second
 * @data_rate:		(*) Desired amount of samples per second.
 *
 * Values listed with (*) shall be provided by the user.
 */
struct eb_dev {
	char ipaddr[17];

	int fd_init;
	int fd_ctrl;
	int fd_data;

	struct eb_client client;
	struct eb_firmware fw_info;
	struct eb_hardware hw_info;

	int packet_rate;
	int data_rate;

	int sample_count;
	struct eb_sample_list *samples;
	struct eb_sample_list *samples_end;
};

struct eb_sample_list {
	struct eb_sample_list *next;
	int seq;
	float eeg[EB_BEPLUSLTM_EEG_CHAN];
	float dc[EB_BEPLUSLTM_DC_CHAN];
};

/* network.c */
int eb_send(int fd, uint8_t pid, const void *buf, uint16_t len);
int eb_send_id(int fd, uint8_t pid);
int eb_recv(int fd, void *buf, uint16_t len, int *err);
int eb_recv_noblock(int fd, void *buf, uint16_t len, int *err);
int eb_recv_err(int fd);
int eb_send_recv_err(int fd, uint8_t pid, const void *buf, uint16_t len);
int eb_request_info(int fd, uint8_t pid, void *buf, uint16_t len, int *err);

int eb_prepare(struct eb_dev *dev);
int eb_set_default_preset(struct eb_dev *dev);
int eb_unprepare(struct eb_dev *dev);
int eb_set_mode(struct eb_dev *dev, int mode);
int eb_get_data(struct eb_dev *dev, float *eeg_buf, float *dc_buf, int sample_cnt);


#endif /* EBNEURO_H` */
