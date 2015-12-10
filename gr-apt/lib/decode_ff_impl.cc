/* -*- c++ -*- */
/* 
 * Copyright 2015 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "decode_ff_impl.h"

#define SAMPLE_RATE 11025

namespace gr {
  namespace apt {

    decode_ff::sptr
    decode_ff::make()
    {
      return gnuradio::get_initial_sptr
        (new decode_ff_impl());
    }

    decode_ff_impl::decode_ff_impl()
      : gr::block("decode_ff",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(1, 1, sizeof(float)))
    {
	    buffer_initialize(&d_signal_buffer, SAMPLE_RATE);
	    buffer_initialize(&d_image_buffer, NUM_PIXELS_IN_ROW*2);
	    apt_initialize(&d_apt);
    }

    decode_ff_impl::~decode_ff_impl()
    {
	    buffer_free(&d_signal_buffer);
	    buffer_free(&d_image_buffer);
    }

    void
    decode_ff_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
	ninput_items_required[0] = noutput_items;
    }

    int
    decode_ff_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        const float *in = (const float *) input_items[0];
        float *out = (float*) output_items[0];

	//write to buffer
	int written_samples = buffer_fill(&d_signal_buffer, ninput_items[0], in);

	//start decoding when buffer is filled
	if (written_samples < ninput_items[0]) {
		float pixel_data[NUM_PIXELS_IN_ROW] = {0};
		int retval = apt_decode(&d_apt, &d_signal_buffer, pixel_data);

		if (retval != 0) {
			buffer_fill(&d_image_buffer, NUM_PIXELS_IN_ROW, pixel_data);
		}
	}
        consume_each (written_samples);

	//check pixel buffer for possible output
	int num_generated_pixels = buffer_read(&d_image_buffer, noutput_items, out);
        return num_generated_pixels;
    }

  } /* namespace apt */
} /* namespace gr */

