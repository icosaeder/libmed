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
	char *driver = "dummy";
	struct med_eeg *dev;
	int i, j, ret;
	char **labels;
	float *data;

	ret = med_eeg_create(&dev, driver, NULL);
	assert(!ret);
	
	printf("Using \"%s\" driver with %d channels.\n", driver, dev->channel_count);
	data = malloc(dev->channel_count * sizeof(*data));

	ret = med_eeg_get_channels(dev, &labels);
	assert(ret > 0);

	for (i = 0; i < ret; ++i)
		printf("%6s ", labels[i]);
	printf("\n");

	for (i = 0; i < 10; ++i) {
		ret = med_eeg_sample(dev, data, 1);
		assert(ret > 0);

		for (j = 0; j < dev->channel_count; ++j)
			printf("%6.1f ", data[j]);
		printf("\n");
	}

	printf("Impedance:\n");

	ret = med_eeg_get_impedance(dev, data);
	assert(ret > 0);

	for (j = 0; j < dev->channel_count; ++j)
		printf("%6.1f ", data[j]);
	printf("\n");

	med_eeg_destroy(dev);
	free(data);

	return 0;
}
