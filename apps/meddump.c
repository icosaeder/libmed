// SPDX-License-Identifier: GPL-3.0-only

/*
 * meddump.c - A tiny cli utility to dump the eeg device data.
 */

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>

#include <med/eeg.h>

volatile sig_atomic_t stop;

void stop_sampling(int signum)
{
	stop = 1;
}

/**
 * print_labels() - Print the device channel labels.
 */
int print_labels(struct med_eeg *dev)
{
	int chan_cnt, ret, i;
	char **labels;

	chan_cnt = med_eeg_get_channels(dev, &labels);
	if (chan_cnt < 0)
		return chan_cnt;

	for (i = 0; i < chan_cnt; ++i)
		printf("%9s ", labels[i]);
	printf("\n");

	return 0;
}

/**
 * sample_loop() - Run a loop that samples the device.
 * @dev:   The device to sample.
 * @count: Amount to samples to receive. Can be -1 to run forever.
 * @mode:  Mode in which to sample the device.
 */
int sample_loop(struct med_eeg *dev, int count, enum med_eeg_mode mode, useconds_t dly)
{
	int chan_cnt, ret, i, j;
	float *data;

	chan_cnt = med_eeg_get_channels(dev, NULL);
	if (chan_cnt < 0)
		return chan_cnt;

	data = malloc(chan_cnt * sizeof(*data));

	ret = med_eeg_set_mode(dev, mode);
	if (ret) {
		fprintf(stderr, "Failed to set mode: %d\n", ret);
		return ret;
	}

	for (i = 0; !stop && (count == -1 || i < count); ++i) {
		switch (mode) {
			case MED_EEG_SAMPLING:
			case MED_EEG_TEST:
				ret = med_eeg_sample(dev, data, 1);
				break;
			case MED_EEG_IMPEDANCE:
				ret = med_eeg_get_impedance(dev, data);
				break;
			default:
				return -EINVAL;
		}
		if (ret < 0) {
			fprintf(stderr, "Failed to get sample: %d\n", ret);
			return ret;
		}

		for (j = 0; j < chan_cnt; ++j)
			printf("% 8.6f ", data[j]);
		printf("\n");
		usleep(dly);
	}

	free(data);

	return 0;
}

void usage(char *pn)
{
	fprintf(stderr, "Usage: %s [-ivh] driver [key=val ...]\n\n", pn);
	fprintf(stderr, " -i      Sample impedance.\n");
	fprintf(stderr, " -t      Sample test signal.\n");
	fprintf(stderr, " -c cnt  Stop after cnt samples.\n");
	fprintf(stderr, " -d dly  Delay each sample by dly ms.\n");
	fprintf(stderr, " -v      Be more verbose.\n");
	fprintf(stderr, " -h      Print this help message.\n");
}

int main(int argc, char *argv[])
{
	enum med_eeg_mode mode = MED_EEG_SAMPLING;
	bool verbose = false;
	struct med_eeg *dev;
	struct med_kv *conf;
	int i, ret, opt, cnt=-1, chan_cnt;
	useconds_t dly = 0;
	char *driver;

	signal(SIGINT, stop_sampling);

	while ((opt = getopt(argc, argv, "itvhc:d:")) != -1) {
		switch (opt) {
			case 'i':
				mode = MED_EEG_IMPEDANCE;
				break;
			case 't':
				mode = MED_EEG_TEST;
				break;
			case 'c':
				cnt = atoi(optarg);
				break;
			case 'd':
				dly = atoi(optarg) * 1000;
				break;
			case 'v':
				verbose = true;
				break;
			case 'h':
				usage(argv[0]);
				exit(0);
			default: /* '?' */
				usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if (optind >= argc) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	driver = argv[optind];
	argv += optind + 1;
	argc -= optind + 1;

	conf = malloc(sizeof(*conf) * (argc + 2));
	for (i = 0; i < argc; ++i) {
		conf[i].key = strtok(argv[i], "=");
		conf[i].val = strtok(NULL, "=");
	}

	conf[i].key = "verbosity";
	conf[i].val = verbose ? "3" : "0";
	conf[i+1].key = NULL;
	conf[i+1].val = NULL;

	ret = med_eeg_create(&dev, driver, conf);
	if (ret) {
		fprintf(stderr, "Failed to create the device: %d\n", ret);
		exit(EXIT_FAILURE);
	}

	chan_cnt = med_eeg_get_channels(dev, NULL);
	if (chan_cnt < 0) {
		fprintf(stderr, "Failed to get the device channels: %d\n", chan_cnt);
		exit(EXIT_FAILURE);
	}
	
	fprintf(stderr, "Using the '%s' driver with %d channels.\n", driver, chan_cnt);

	print_labels(dev);

	ret = sample_loop(dev, cnt, mode, dly);
	if (ret)
		fprintf(stderr, "Failed to get a sample: %d\n", ret);

	ret = med_eeg_set_mode(dev, MED_EEG_IDLE);
	if (ret)
		fprintf(stderr, "Failed to set idle mode: %d\n", ret);

	med_eeg_destroy(dev);
	free(conf);

	return 0;
}
