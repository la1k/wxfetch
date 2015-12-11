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

#ifndef INCLUDED_APT_IMAGE_WIDGET_F_IMPL_H
#define INCLUDED_APT_IMAGE_WIDGET_F_IMPL_H

#include <apt/image_widget_f.h>
#include <opencv2/core/core.hpp>
#include "image_viewer.h"

namespace gr {
  namespace apt {
    class image_widget_f_impl : public image_widget_f
    {
     private:
      cv::Mat d_img;
      ImageViewer *imageViewer;

     public:
      image_widget_f_impl();
      ~image_widget_f_impl();
      virtual QWidget* qwidget(){return imageViewer;};

      int work(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
    };

  } // namespace apt
} // namespace gr

#endif /* INCLUDED_APT_IMAGE_WIDGET_F_IMPL_H */

