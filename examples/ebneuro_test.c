// SPDX-License-Identifier: GPL-3.0-only

/* ebneuro_test.c - minimal usage example for the ebneuro class. */

#include <stdio.h>

#include <ebneuro.h>

int main(int argc, char *argv[])
{
	int ret, i, j, k;
	struct eb_dev dev = { 
		.ipaddr = "192.168.171.81",
		.packet_rate = 64,
		.data_rate = 512,
	};
	float eeg_data[64 * 1] = {0};
	float dc_data[4 * 1] = {0};
	short eeg_imp[64] = {0};
	short dc_imp[4] = {0};


	ret = eb_prepare(&dev);
	if (ret) {
		printf("Prepare failed! %d\n", ret);
		return 1;
	}

	ret = eb_set_default_preset(&dev);
	if (ret) {
		printf("Preset upload failed! %d\n", ret);
		goto error;
	}

	ret = eb_set_mode(&dev, EB_MODE_SAMPLE);
	if (ret) {
		printf("Setting sampling mode failed! %d\n", ret);
		goto error;
	}

	printf("Press any key...\n");
	getchar();

	for (i = 0; i < 20; ++i) {
		ret = eb_get_data(&dev, eeg_data, dc_data, 1);
		printf("%5d; %5d) \n", i, ret);
		for (j = 0; j < 8; ++j) {
			for (k = 0; k < 8; ++k)
				printf("%8.2f ", eeg_data[j*8 + k]);
			printf("\n");
		}

		printf("\n");
		for (j = 0; j < 4; ++j)
			printf("%9.2f ", dc_data[j]);

		printf("\n--------------------\n");
	}

	ret = eb_set_mode(&dev, EB_MODE_IMPEDANCE);
	if (ret) {
		printf("Setting impedance mode failed! %d\n", ret);
		goto error;
	}

	for (i = 0; i < 20; ++i) {
		ret = eb_get_impedances(&dev, &eeg_imp[0], &dc_imp[0]);
		printf("%5d; %5d\n", i, ret);

		for (j = 0; j < 8; ++j) {
			for (k = 0; k < 8; ++k)
				printf("%8d ", eeg_imp[j*8 + k]);
			printf("\n");
		}

		printf("\n");
		for (j = 0; j < 4; ++j)
			printf("%8d ", dc_imp[j]);

		printf("\n--------------------\n");
	}

error:
	ret = eb_unprepare(&dev);
	if (ret) {
		printf("Unrepare failed! %d\n", ret);
		return 1;
	}

	return !!ret;
}
