/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef MED_DRIVERS_H
#define MED_DRIVERS_H

int dummy_create(struct med_eeg **dev, struct med_kv *kv);
int ebneuro_create(struct med_eeg **edev, struct med_kv *kv);
int openbci_create(struct med_eeg **edev, struct med_kv *kv);

#endif /* MED_DRIVERS_H */
