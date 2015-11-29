/* -*- c++ -*- */

#define APT_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "apt_swig_doc.i"

%{
#include "apt/decode_ff.h"
%}


%include "apt/decode_ff.h"
GR_SWIG_BLOCK_MAGIC2(apt, decode_ff);
