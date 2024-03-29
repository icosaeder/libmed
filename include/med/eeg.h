/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef LIBMED_EEG_H
#define LIBMED_EEG_H

/**
 * enum med_eeg_mode - Modes of operation for an EEG device.
 *
 * @MED_EEG_IDLE:	Idle mode.
 * @MED_EEG_SAMPLING:	Active EEG sampling mode.
 * @MED_EEG_IMPEDANCE:	EEG Impedance measurement mode.
 * @MED_EEG_TEST:	Test data generated by the device.
 */
enum med_eeg_mode {
	MED_EEG_IDLE,
	MED_EEG_SAMPLING,
	MED_EEG_IMPEDANCE,
	MED_EEG_TEST,
};

/**
 * struct med_kv - Key-Value configuration pair.
 */
struct med_kv {
	const char *key;
	const char *val;
};

/**
 * struct med_eeg - EEG Device.
 */
struct med_eeg;

/**
 * med_eeg_create() - Construct and preconfigure an EEG device.
 * @dev:  Pointer to return the device instance to.
 * @type: Type of the device to create.
 * @kv:   Key-Value pairs that configure the device.
 *        Note that the list must be terminated by an
 *        empty KV.
 *
 * This methods picks a driver to probe for the given device
 * type and passes the KV list to it. If successful, the @dev
 * is populated with a pointer to the new EEG device object
 * to be used with other methods.
 *
 * Return: Zero on success and negative error otherwise.
 */
int med_eeg_create(struct med_eeg **dev, char *type, struct med_kv *kv);

/**
 * med_eeg_destroy() - Unprepare the deivce and free the used resources.
 * @dev:  The device to destroy.
 */
void med_eeg_destroy(struct med_eeg *dev);

/**
 * med_eeg_set_mode() - Set mode of the device.
 * @dev:  The EEG device to act on.
 * @mode: The mode to switch the device into.
 *
 * This method switches the device into one of the available modes.
 * The support for each mode depends on the specific driver used.
 *
 * Return: Zero on success or a negative error otherwise.
 */
int med_eeg_set_mode(struct med_eeg *dev, enum med_eeg_mode mode);

/**
 * med_eeg_get_channels() - Read the available channel labels.
 * @dev:    The EEG device to act on.
 * @labels: Pointer to write a label array to.
 *
 * This method returns driver-specific labels for the data.
 *
 * Return: Amount of channel labels or a negative error number.
 */
int med_eeg_get_channels(struct med_eeg *dev, char ***labels);

/**
 * ed_eeg_sample() - Read samples blocking.
 * @dev:     The device to read from.
 * @samples: Pointer to the buffer to fill with the data.
 * @count:   Amount of samples to read.
 *
 * This method will block until the internal buffer has
 * @count samples and will copy the values into @samples.
 * The @samples buffer size must fit
 *	(sizeof(float) * dev->channel_count * count)
 *
 * The new samples will be received during this call.
 * The driver will be called to receive data even if the
 * @count is zero. In this case the data will be queued
 * but no samples will be written out.
 *
 * Returns: Amount of values read or a negative error.
 */
int med_eeg_sample(struct med_eeg *dev, float *samples, int count);

/**
 * med_get_impedance() - Read impedances.
 * @dev: The device to read from.
 * @samples: Pointer to the buffer to fill with the data.
 * 
 * This method blocks and reads out a single set of impedance
 * values into @samples. The buffer must fit
 *	(sizeof(float) * dev->channel_count)
 *
 * If only a subset of channels supports impedance detection,
 * the remaining channels will contain a NaN value.
 *
 * Returns: Amount of values read or a negative error.
 */
int med_eeg_get_impedance(struct med_eeg *dev, float *samples);

#endif /* LIBMED_EEG_H */
