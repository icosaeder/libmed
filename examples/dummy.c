// SPDX-License-Identifier: GPL-3.0-only

/*
 * dummy.c - A minimal usage example with the dummy driver
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <med/eeg.h>

int main(int argc, char *argv[])
{
	int i, j, ret, chan_cnt;
	char *driver = "dummy";
	struct med_eeg *dev;
	struct med_kv conf[] = {{"channels", "8"}, {"foo", "bar"}, {0}};
	char **labels;
	float *data;

	ret = med_eeg_create(&dev, driver, conf);
	assert(!ret);
	
	chan_cnt = med_eeg_get_channels(dev, &labels);
	assert(chan_cnt > 0);

	printf("Using \"%s\" driver with %d channels.\n", driver, chan_cnt);
	data = malloc(chan_cnt * sizeof(*data));

	for (i = 0; i < chan_cnt; ++i)
		printf("%6s ", labels[i]);
	printf("\n");

	ret = med_eeg_set_mode(dev, MED_EEG_SAMPLING);
	assert(!ret);

	for (i = 0; i < 10; ++i) {
		ret = med_eeg_sample(dev, data, 1);
		assert(ret > 0);

		for (j = 0; j < chan_cnt; ++j)
			printf("%6.1f ", data[j]);
		printf("\n");
	}

	printf("Impedance:\n");

	ret = med_eeg_set_mode(dev, MED_EEG_IMPEDANCE);
	assert(!ret);

	ret = med_eeg_get_impedance(dev, data);
	assert(ret > 0);

	for (j = 0; j < chan_cnt; ++j)
		printf("%6.1f ", data[j]);
	printf("\n");

	ret = med_eeg_set_mode(dev, MED_EEG_IDLE);
	assert(!ret);

	med_eeg_destroy(dev);
	free(data);

	return 0;
}
