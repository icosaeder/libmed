/* SPDX-License-Identifier: GPL-3.0-only */
%module pyebneuro

%include "exception.i"
%include "carrays.i"
%array_class(float, floatArray);
%array_class(short, shortArray);

%{
#define SWIG_FILE_WITH_INIT
#include <ebneuro.h>
%}

%include "../packets.h"
%include <ebneuro.h>

%extend eb_dev {
        int _prepare() { return eb_prepare($self); }
        int _unprepare() { return eb_unprepare($self); }
        int _set_mode(int mode) { return eb_set_mode($self, mode); }
        int _set_preset(int packet_rate, int data_rate)
        {
                $self->packet_rate = packet_rate;
                $self->data_rate = data_rate;
                return eb_set_default_preset($self);
        }
        int _get_data(float *eeg_buf, float *dc_buf, int sample_cnt)
        {       return eb_get_data($self, eeg_buf, dc_buf, sample_cnt); }
        int _get_impedances(short *eeg, short *dc)
        {       return eb_get_impedances($self, eeg, dc); }
}

/* TODO: Need to rewrite this stuff to use the typemaps and exception magic */
%pythoncode %{
class Ebneuro:
        def __init__(self, ipaddr):
                self._dev = eb_dev()
                self._dev.ipaddr = ipaddr
                self._dev.packet_rate = 64
                self._dev.data_rate = 512

        def prepare(self):
                ret = self._dev._prepare()
                if ret != 0:
                        raise Exception(f"failed with {ret}")

        def unprepare(self):
                ret = self._dev._unprepare()
                if ret != 0:
                        raise Exception(f"failed with {ret}")

        def set_mode(self, mode):
                ret = self._dev._set_mode(mode)
                if ret != 0:
                        raise Exception(f"failed with {ret}")

        def set_preset(self, packet_rate, data_rate):
                ret = self._dev._set_preset(packet_rate, data_rate)
                if ret != 0:
                        raise Exception(f"failed with {ret}")

        def get_data(self, count):
                eeg = floatArray(count * EB_BEPLUSLTM_EEG_CHAN)
                dc = floatArray(count * EB_BEPLUSLTM_DC_CHAN)
                ret = self._dev._get_data(eeg, dc, count)
                if ret < 0:
                        raise Exception(f"failed with {ret}")
                return ([ [ eeg[EB_BEPLUSLTM_EEG_CHAN*smp + i]
                                for i in range(EB_BEPLUSLTM_EEG_CHAN) ]
                                        for smp in range(count) ],
                        [ [ dc[EB_BEPLUSLTM_DC_CHAN*smp + i]
                                for i in range(EB_BEPLUSLTM_DC_CHAN) ]
                                        for smp in range(count) ])

        def get_impegances(self):
                eeg = floatArray(EB_BEPLUSLTM_EEG_CHAN)
                dc = floatArray(EB_BEPLUSLTM_DC_CHAN)
                ret = self._dev._get_impedances(eeg, dc)
                if ret != 0:
                        raise Exception(f"failed with {ret}")
                return ([ eeg[i] for i in range(EB_BEPLUSLTM_EEG_CHAN) ],
                        [ dc[i] for i in range(EB_BEPLUSLTM_DC_CHAN) ])

%}
