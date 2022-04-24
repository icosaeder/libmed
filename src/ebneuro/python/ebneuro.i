/* SPDX-License-Identifier: GPL-3.0-only */
%module pyebneuro

%include "exception.i"
%include "carrays.i"
%array_class(float, floatArray);

%{
#define SWIG_FILE_WITH_INIT
#include <ebneuro.h>
%}

%include "../packets.h"
%include <ebneuro.h>

%extend eb_dev {
        int prepare() { return eb_prepare($self); }
        int unprepare() { return eb_unprepare($self); }
        int set_mode(int mode) { return eb_set_mode($self, mode); }
        int set_preset(int packet_rate, int data_rate)
        {
                $self->packet_rate = packet_rate;
                $self->data_rate = data_rate;
                return eb_set_default_preset($self);
        }
}

