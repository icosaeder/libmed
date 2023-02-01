/* SPDX-License-Identifier: GPL-3.0-only */
%module pylibmed

%{
#define SWIG_FILE_WITH_INIT
#include <med/eeg.h>
%}

%include "numpy.i"

/*
 * Input kv parameters.
 */
%typemap(in, numinputs=1)
  (struct med_kv *kv)
{
	if ($input != Py_None) {
		if (!PyDict_Check($input)) {
			PyErr_Format(PyExc_TypeError, "Expected dict.");
			SWIG_fail;
		}

		Py_ssize_t len = PyDict_Size($input);
		$1 = malloc(sizeof(struct med_kv) * len);

		PyObject *key, *value;
		Py_ssize_t pos = 0;

		while (PyDict_Next($input, &pos, &key, &value)) {
			if (!PyUnicode_Check(key)) {
				PyErr_Format(PyExc_TypeError, "Expected key to be str.");
				SWIG_fail;
			}
			/*
			 * NOTE: This seem to produce a warning about memory leak.
			 * The strings created here should be destroyed when the
			 * str object is GC'd though...
			 */
			$1[pos-1].key = PyUnicode_AsUTF8(key);
			$1[pos-1].val = PyUnicode_AsUTF8(PyObject_Str(value));
		}
		$1[len].key = NULL;
	}
}
%typemap(argout)
  (struct med_kv *kv)
{
	if ($input != Py_None)
		free($1);
}

/*
 * Output labels.
 */
%typemap(in, numinputs=0)
  (char ***labels)
  (char **lbs = NULL)
{
	$1 = &lbs;
}
%typemap(argout)
  (char ***labels)
{
	$result = NULL;
	for (int i = 0; i < result; i++)
		$result = SWIG_Python_AppendOutput($result, PyUnicode_FromString(lbs$argnum[i]));
}

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
