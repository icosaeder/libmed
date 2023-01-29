/* SPDX-License-Identifier: GPL-3.0-only */
%module pylibmed

%{
#define SWIG_FILE_WITH_INIT
#include <med/eeg.h>
%}

%include "numpy.i"

%include <med/eeg.h>

%rename(Eeg) med_eeg;
%rename("%(regex:/^(med_.*)/_\\1/)s") "";

%extend med_eeg {
	~med_eeg() {
		med_eeg_destroy($self);
	}
	int set_mode(enum med_eeg_mode mode);
	int get_channels(char ***labels=NULL);
	int sample(float *samples=NULL, int count=0);
	int get_impedance(float *samples);
}


%extend med_eeg {
	med_eeg(char *type, struct med_kv *kv) {
		struct med_eeg *dev;
		int ret = med_eeg_create(&dev, type, kv);
		return dev;
	}
}

/* Let SWIG treat it as declared */
struct med_eeg {};
