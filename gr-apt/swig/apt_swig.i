/* -*- c++ -*- */

#define APT_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "apt_swig_doc.i"

%{
#include "apt/decode_ff.h"
#include "apt/image_file_f.h"
#include "apt/image_widget_f.h"
%}


%include "apt/decode_ff.h"
GR_SWIG_BLOCK_MAGIC2(apt, decode_ff);
%include "apt/image_file_f.h"
GR_SWIG_BLOCK_MAGIC2(apt, image_file_f);
%include "apt/image_widget_f.h"
GR_SWIG_BLOCK_MAGIC2(apt, image_widget_f);
