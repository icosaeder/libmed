// SPDX-License-Identifier: GPL-3.0-only

/*
 * impadance.c - Driver side impedance calculation.
 */

#include <assert.h>
#include <string.h>

#include <system/system.h>

#include "openbci.h"
#include "packets.h"
#include "OpenBCI_32bit_Library_Definitions.h"

/**
 * obci_calculate_leadoff_impedane() - Perform a calculation of impedances based on the collected buffer.
 */
int obci_calculate_leadoff_impedane(struct med_eeg *edev, float *samples, float *impedances)
{
	int i;



	return 0;
}
