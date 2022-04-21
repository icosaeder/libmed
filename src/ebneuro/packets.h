/* SPDX-License-Identifier: GPL-3.0-only */

/*
 * packets.h - data packet descriptions for the device.
 */

#ifndef PACKETS_H
#define PACKETS_H

#include <stdint.h>

#include "../system/endiannes.h"
#include "../system/helpers.h"

#define EB_SOCK_PORT_INIT 7023
#define EB_SOCK_PORT_CTRL 7024
#define EB_SOCK_PORT_DATA 7025

#define EB_BEPLUSLTM_EEG_CHAN 64
#define EB_BEPLUSLTM_DC_CHAN 4

#define EB_PACKET_START_MAGIC	0x02
#define EB_PACKET_END_MAGIC	0x03

/* Init packet IDs */
#define EB_IPK_ID_DEVICE	8
#define EB_IPK_ID_CLIENT	10
#define EB_IPK_ID_CLIENT_SET	11
#define EB_IPK_ID_FIRMWARE	53
#define EB_IPK_ID_HARDWARE	1
#define EB_IPK_ID_SET_SOCK	9

/* Control packet IDs */
#define EB_CPK_ID_IMPEDANCE	47
#define EB_CPK_ID_PRESET_UPL	16
#define EB_CPK_ID_MODE_SET	20
#define EB_CPK_ID_STIM		92
#define EB_CPK_ID_SUPPORT_SET	18
#define EB_CPK_ID_WIRELESS	64

/* Data packet IDs */
#define EB_DPK_ID_DATA		1
#define EB_DPK_ID_ARCHIVE	4
#define EB_DPK_ID_ENDARCHIVE	5

struct eb_packet_hdr {
	uint8_t magic;
	uint8_t id;
	__be16 length;
};

#define EB_SOCK_ENABLED		0
#define EB_SOCK_DISABLED	1
#define EB_SOCK_CONNECTED	4
#define EB_SOCK_DISCONNECTED	5

struct eb_socket {
	__le16 enabled;
	__le16 connected;
	uint8_t ipaddr[16];
	__le16 port;
	uint8_t reserved[2];
};

struct eb_client {
	struct eb_socket init_sock;
	struct eb_socket control_sock;
	struct eb_socket data_sock;
	__le32 status;
	uint8_t name[32];
	uint8_t reserved[20];
};

struct eb_client_set {
	__le32 timestamp;
	uint8_t name[32];
	uint8_t reserved[28];
};

struct eb_firmware {
	__le16 release;
	__le16 build;
	__le16 dsp_release;
	__le16 dsp_build;
	__le16 epld_release;
	uint8_t reserved[118];
};

/* Capabilities */
#define	EB_CAP_EEG		BIT(0)
#define	EB_CAP_WIFI		BIT(1)
#define	EB_CAP_USB		BIT(2)
#define	EB_CAP_SUPPORT_ADVANCED	BIT(4)
#define	EB_CAP_SUPPORT_FULL	BIT(5)
#define	EB_CAP_SUPPORT_STANDARD	BIT(6)
#define	EB_CAP_SUPPORT_LIGHT	BIT(7)

struct eb_hardware {
	__le32 prod_time;
	__le32 sku;
	__le32 prod_number;
	__le32 serial;
	uint8_t digital_board_sn[16];
	uint8_t analog_board_sn[16];
	uint8_t wireless_sn[16];
	__le32 oem;
	__le32 hw_rev;
	__le32 battery_sku;
	uint8_t reserved[32];
	__le16 btn_shutdown_delay; /* sec */
	__le16 shutdown_timeout; /* sec */
	__le32 capabilities;
};

#define EB_SOCK_INDEX_CTRL	0
#define EB_SOCK_INDEX_DATA	1

#define EB_SOCK_STATE_ENABLE	0
#define EB_SOCK_STATE_DISABLE	1
#define EB_SOCK_STATE_STOP	2
#define EB_SOCK_STATE_START	3
#define EB_SOCK_STATE_CONNECTED	4
#define EB_SOCK_STATE_DISCONNECTED 5

struct eb_sock_state {
	__le16 index;
	__le16 state;
};

#define EB_FLAG_OHM_SIGNAL	BIT(1)
#define EB_FLAG_STIM_MONITOR	BIT(3)
#define EB_FLAG_CASCADE		BIT(4)
#define EB_FLAG_MASTER		BIT(5)
#define EB_FLAG_PULSE_OXY	BIT(6)

struct eb_preset {
	uint8_t name[32];
	__le16 eeg_rates[EB_BEPLUSLTM_EEG_CHAN];
	__le16 dc_rates[EB_BEPLUSLTM_DC_CHAN];
	__le16 flags;
	__le16 flags2;
	__le16 mains_rate;
	__le16 packet_rate;
	uint8_t reserved[128];
};

#define EB_MODE_IDLE		0
#define EB_MODE_SAMPLE		1
#define EB_MODE_WAVE		2
#define EB_MODE_IMPEDANCE	3

struct eb_mode {
	__le16 mode;
	uint8_t reserved[2];
};

struct eb_device {
	uint8_t name[32];
	__le16 index;
};

struct eb_stim {
	__le16 rate;  /* signed */
	__le16 pulse; /* signed */
};

#define EB_SUPPORT_BACKUP	BIT(0)
#define EB_SUPPORT_CHARGER	BIT(1)

struct eb_support {
	__le16 flags;
	uint8_t reserved[14];
};

#define EB_IMP_NONE	-1
#define EB_IMP_OPEN	-2

struct eb_imp {
	__le16 p;
	__le16 n;
};

struct eb_impedance_info {
	struct eb_imp eeg[EB_BEPLUSLTM_EEG_CHAN];
	struct eb_imp dc[EB_BEPLUSLTM_DC_CHAN];
	struct eb_imp ref;
	struct eb_imp gnd;
};

#endif /* PACKETS_H */
