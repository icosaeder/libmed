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

/*
 * Output eeg samples.
 */
%typemap(in, numinputs=1, fragment="NumPy_Fragments")
  (float *samples, int count)
  (PyObject* val_arr = NULL)
{
	if (!PyLong_Check($input)) {
		PyErr_Format(PyExc_TypeError, "Expected number but '%s' given.",
				pytype_string($input));
		SWIG_fail;
	}

	$2 = PyLong_AsLong($input);
	if ($2 == -1 && PyErr_Occurred())
                SWIG_fail;

	struct med_eeg *dev;
	int res = SWIG_ConvertPtr($self, (void**)&dev, $descriptor(struct med_eeg*), 0);
	if (!SWIG_IsOK(res)) {
		SWIG_exception_fail(SWIG_ArgError(res), "in method '" "$symname" "', argument "
				"$argnum"" of type '" "$type""'");
	}

	int slen = med_eeg_get_channels(dev, NULL);

	val_arr = PyArray_SimpleNew(2, ((npy_intp []){$2, slen}), NPY_FLOAT);
	if (!val_arr)
		SWIG_fail;

	$1 = (float*) array_data(val_arr);
}
%typemap(argout)
  (float *samples, int count)
{
	$result = val_arr$argnum;
}

/*
 * Output eeg impedances.
 */
%typemap(in, numinputs=0, fragment="NumPy_Fragments")
  (float *samples)
  (PyObject* val_arr = NULL)
{
	struct med_eeg *dev;
	/*
	 * HACK: The "args" here comes from SWIG wrapper function arguments.
	 * It's a PyObject "tuple" with only one value, making it the "$self"
	 * For whatever reason this code is inserted before the real $self is
	 * initialized if the numinputs is 0. All of this is kinda fishy but
	 * I hope it doesn't explode...
	 */
	int res = SWIG_ConvertPtr(args, (void**)&dev, $descriptor(struct med_eeg*), 0);
	if (!SWIG_IsOK(res)) {
		SWIG_exception_fail(SWIG_ArgError(res), "in method '" "$symname" "', argument "
				"$argnum"" of type '" "$type""'");
	}

	int slen = med_eeg_get_channels(dev, NULL);

	val_arr = PyArray_SimpleNew(1, ((npy_intp []){slen}), NPY_FLOAT);
	if (!val_arr)
		SWIG_fail;

	$1 = (float*) array_data(val_arr);
}
%typemap(argout)
  (float *samples)
{
	$result = val_arr$argnum;
}

%init %{
    import_array();
%}

%include <med/eeg.h>

%rename(Eeg) med_eeg;

%exception ~med_eeg;
%exception med_eeg {
        $action
        if (!result)
                SWIG_fail;
}
%exception {
        $action
        if (result < 0) {
                PyErr_Format(PyExc_Exception, "Invocation failed: %d", result);
                SWIG_fail;
        }
}

%extend med_eeg {
	int set_mode(enum med_eeg_mode mode);
	int get_channels(char ***labels=NULL);
	int sample(float *samples=NULL, int count=0);
	int get_impedance(float *samples);
}

%extend med_eeg {
	med_eeg(char *type, struct med_kv *kv) {
		struct med_eeg *dev;
		int ret = med_eeg_create(&dev, type, kv);
		if (ret)
			PyErr_Format(PyExc_Exception, "Failed to create: %d.", ret);
		return dev;
	}
	~med_eeg() {
		med_eeg_destroy($self);
	}
}

%exception; /* Don't apply the handler to anything else */

/* Let SWIG treat it as declared */
struct med_eeg {};
