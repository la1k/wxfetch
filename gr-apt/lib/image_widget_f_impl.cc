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
#include "image_widget_f_impl.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <apt.h>
#include <iostream>

namespace gr {
  namespace apt {

    image_widget_f::sptr
    image_widget_f::make()
    {
      return gnuradio::get_initial_sptr
        (new image_widget_f_impl());
    }

    image_widget_f_impl::image_widget_f_impl()
      : gr::sync_block("image_widget_f",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(0, 0, 0))
    {
      gr::block::set_output_multiple(NUM_PIXELS_IN_ROW);
      imageViewer = new ImageViewer;
    }

    image_widget_f_impl::~image_widget_f_impl()
    {
    }

    int
    image_widget_f_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const float *in = (const float*) input_items[0];

	//generate cv::Mat from input data
	float *img_data = new float[noutput_items]();
	memcpy(img_data, in, sizeof(float)*noutput_items);
	int num_rows = noutput_items/NUM_PIXELS_IN_ROW;
	int displayed_elements = num_rows*NUM_PIXELS_IN_ROW;
	cv::Mat rows(num_rows, NUM_PIXELS_IN_ROW, CV_32FC1, img_data);
	delete [] img_data;

	//convert to RGB-format grayscale image
	cv::Mat uchar_image;
	rows.convertTo(uchar_image, CV_8UC1);

	std::vector<cv::Mat> channels;
	channels.push_back(uchar_image.clone());
	channels.push_back(uchar_image.clone());
	channels.push_back(uchar_image.clone());
	
	cv::Mat rgb_img;
	cv::merge(channels, rgb_img);

	//display image data
	imageViewer->updateImage_fromNonGUI(rgb_img.clone());

	consume_each(noutput_items);
        return 0;
    }

  } /* namespace apt */
} /* namespace gr */

