#ifndef EBNEURO_H
#define EBNEURO_H

#include <stdint.h>

#include "../system/system.h"
#include "../system/endiannes.h"
#include "../system/helpers.h"

struct eb_dev {
	char ipaddr[17];

	int fd_init;
	int fd_ctrl;
	int fd_data;

	struct eb_client client;
	struct eb_firmware fw_info;
	struct eb_hardware hw_info;
};

/* network.c */
int eb_send(int fd, uint8_t pid, const void *buf, uint16_t len);
int eb_send_id(int fd, uint8_t pid);
int eb_recv(int fd, void *buf, uint16_t len, int *err);
int eb_recv_err(int fd);
int eb_send_recv_err(int fd, uint8_t pid, const void *buf, uint16_t len);
int eb_request_info(int fd, uint8_t pid, void *buf, uint16_t len, int *err)

#endif /* EBNEURO_H */
