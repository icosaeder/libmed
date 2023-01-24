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
	struct med_eeg *dev;
	float data[4*3];
	int i, j, k, ret;

	ret = med_eeg_create(&dev, "dummy", NULL);
	assert(!ret);

	for (i = 0; i < 10; ++i) {
		ret = med_eeg_sample(dev, data, 3);
		assert(ret >= 0);
		for (j = 0; j < 3; ++j) {
			for (k = 0; k < 4; ++k) {
				printf("%5.1f ", data[j+k]);
			}
			printf("| ");
		}
		printf("\n");
	}

	med_eeg_destroy(dev);


	return 0;
}
