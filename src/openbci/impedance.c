// SPDX-License-Identifier: GPL-3.0-only

/*
 * impadance.c - Driver side impedance calculation.
 */

#include <assert.h>
#include <string.h>
#include <math.h>

#include <system/system.h>

#include "openbci.h"
#include "packets.h"
#include "OpenBCI_32bit_Library_Definitions.h"

/**
 * obci_calculate_leadoff_impedane() - Perform a calculation of impedances based on the collected buffer.
 * @samples:     Data from the adc, sizeof(float * @cnt * @channels)
 * @impedances:  Output buffer to write impedances to.
 * @cnt:         Amount of samples in the input
 * @channels:    Amount of channels in the each input.
 *
 * Calculate impedances from the known current forced via the channels.
 */
int obci_calculate_leadoff_impedane(float *samples, float *impedances, int cnt, int channels)
{
	const float i_rms = OPENBCI_LEAD_OFF_DRIVE_AMPS / sqrt(2);
	float *vmax, *vmin;
	int i, j;

	/*
	 * Calculate RMS voltage over all the given samples.
	 * The board has a 2.2K Ohm resistor on each port.
	 */

	memset(impedances, 0, channels * sizeof(*impedances));

	vmax = malloc(sizeof(*vmax) * channels);
	memset(vmax, 0, sizeof(*vmax) * channels);

	vmin = malloc(sizeof(*vmin) * channels);
	memset(vmin, 0, sizeof(*vmin) * channels);

	/* Vpp = Vmax - Vmin */

	for (i = 0; i < cnt; ++i) {
		for (j = 0; j < channels; ++j) {
			float sample = samples[i*channels + j];
			if (sample > vmax[j])
				vmax[j] = sample;
			if (sample < vmin[j])
				vmin[j] = sample;
		}
	}

	/* Vrms = Vpp / 2sqrt(2) */

	for (i = 0; i < channels; ++i)
		impedances[i] = (vmax[i] - vmin[i]) / (2 * sqrt(2));

	/* R = Vrms / Irms */

	for (i = 0; i < channels; ++i)
		impedances[i] = impedances[i] / i_rms - 2200;

	free(vmax);
	free(vmin);

	return channels;
}
