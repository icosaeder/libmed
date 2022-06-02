/* SPDX-License-Identifier: GPL-3.0-only */
%module pyebneuro

%{
#define SWIG_FILE_WITH_INIT
#include <ebneuro.h>
%}

%include "numpy.i"

/* Typemap for eeg sample arrays */
%typemap(in, numinputs=1, fragment="NumPy_Fragments")
  (float *eeg_buf, float *dc_buf, int sample_cnt)
  (PyObject* eeg_arr = NULL, PyObject* dc_arr = NULL)
{
        /* Check that the input argument is a number */
        if (!PyLong_Check($input)) {
                PyErr_Format(PyExc_TypeError, "Expected number but '%s' given.",
                                pytype_string($input));
                SWIG_fail;
        }

        /* Convert the record count to int and pass to the wrapped function */
        $3 = (int) PyLong_AsSsize_t($input);
        if ($3 == -1 && PyErr_Occurred())
                SWIG_fail;

        /* Create arrays to be filled and pass them on */
        eeg_arr = PyArray_SimpleNew(2, ((npy_intp []){$3, EB_BEPLUSLTM_EEG_CHAN}), NPY_FLOAT);
        dc_arr = PyArray_SimpleNew(2, ((npy_intp []){$3, EB_BEPLUSLTM_DC_CHAN}), NPY_FLOAT);
        if (!eeg_arr || !dc_arr)
                SWIG_fail;

        $1 = (float*) array_data(eeg_arr);
        $2 = (float*) array_data(dc_arr);
}
%typemap(argout)
(float *eeg_buf, float *dc_buf, int sample_cnt)
{
        /* Append (now filled) arrays to the return */
        /* FIXME: Default behavior is to produce a list */
        $result = SWIG_Python_AppendOutput(NULL,(PyObject*)eeg_arr$argnum);
        $result = SWIG_Python_AppendOutput($result,(PyObject*)dc_arr$argnum);
}

/* Typemap for impedance sample arrays */
%typemap(in, numinputs=0, fragment="NumPy_Fragments")
  (short *eeg, short *dc)
  (PyObject* eeg_arr = NULL, PyObject* dc_arr = NULL)
{
        /* Create arrays to be filled and pass them on */
        eeg_arr = PyArray_SimpleNew(1, ((npy_intp []){EB_BEPLUSLTM_EEG_CHAN}), NPY_SHORT);
        dc_arr = PyArray_SimpleNew(1, ((npy_intp []){EB_BEPLUSLTM_DC_CHAN}), NPY_SHORT);
        if (!eeg_arr || !dc_arr)
                SWIG_fail;

        $1 = (short*) array_data(eeg_arr);
        $2 = (short*) array_data(dc_arr);
}
%typemap(argout)
  (short *eeg, short *dc)
{
        /* Append (now filled) arrays to the return */
        $result = SWIG_Python_AppendOutput(NULL,(PyObject*)eeg_arr$argnum);
        $result = SWIG_Python_AppendOutput($result,(PyObject*)dc_arr$argnum);
}

%init %{
    import_array();
%}

/* Cleanup the namespace */
%rename(Ebneuro) eb_dev;
%rename("%(regex:/^(eb_.*)/_\\1/)s") "";
%rename("%(regex:/^(EB_(CPK|DPK|IPK|PACKET|SOCK).*)/_\\0/)s") "";

%include "../packets.h"
%include <ebneuro.h>

/* Exception handler will be applied to the following declarations */
%exception {
        $action
        if (result) {
                PyErr_Format(PyExc_Exception, "Invocation failed: %d", result);
                SWIG_fail;
        }
}

%extend eb_dev {
        int prepare() { return eb_prepare($self); }
        int unprepare() { return eb_unprepare($self); }
        int set_mode(int mode) { return eb_set_mode($self, mode); }
        int set_preset(int packet_rate, int data_rate)
        {       return eb_set_preset($self, packet_rate, data_rate); }
        int get_data(float *eeg_buf, float *dc_buf, int sample_cnt)
        {       return eb_get_data($self, eeg_buf, dc_buf, sample_cnt); }
        int get_impedances(short *eeg, short *dc)
        {       return eb_get_impedances($self, eeg, dc); }
}

%exception; /* Don't apply the handler to anything else */

/* Add a simple constructor */
%extend eb_dev {
        eb_dev(char ipaddr[17]) {
                struct eb_dev *dev = malloc(sizeof(*dev));
                memset(dev, 0, sizeof(*dev));
                strcpy(dev->ipaddr, ipaddr);
                return dev;
        }
}

