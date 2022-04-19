#include <stdio.h>

#include "system/system.h"
#include "ebneuro/ebneuro.h"

int main(int argc, char *argv[])
{
	s_set_verbosity(SPEW);
	s_dprintf(ALWAYS, "hello\n");


	struct eb_dev dev = { 
		.ipaddr = "192.168.171.81",
		.packet_rate = 64,
		.data_rate = 512,
	};

	int ret, i, j;
	ret = eb_prepare(&dev);
	s_dprintf(ALWAYS, "prep -> %d\n", ret);

	ret = eb_set_mode(&dev, EB_MODE_IDLE);
	s_dprintf(ALWAYS, "set mode -> %d\n", ret);

	ret = eb_set_default_preset(&dev);
	s_dprintf(ALWAYS, "set preset -> %d\n", ret);

	ret = eb_set_mode(&dev, EB_MODE_SAMPLE);
	s_dprintf(ALWAYS, "set mode -> %d\n", ret);


	s_dprintf(ALWAYS, "Press any key...\n");
	getchar();

	float eeg_data[64 * 1] = {0};
	float dc_data[4 * 1] = {0};

	for (i = 0; i < 20; ++i) {
		ret = eb_get_data(&dev, eeg_data, dc_data, 1);
		s_dprintf(ALWAYS, "%5d; %5d) ", i, ret);
		for (j = 0; j < 10; ++j)
			s_dprintf(ALWAYS, "%8.2f ", eeg_data[j]);

		s_dprintf(ALWAYS, " | ");

		for (j = 0; j < 4; ++j)
			s_dprintf(ALWAYS, "%9.2f ", dc_data[j]);

		s_dprintf(ALWAYS, "\n");
	}

	ret = eb_set_mode(&dev, EB_MODE_IDLE);
	s_dprintf(ALWAYS, "set preset -> %d\n", ret);

	ret = eb_unprepare(&dev);
	s_dprintf(ALWAYS, "unp -> %d\n", ret);

	return 0;
}
