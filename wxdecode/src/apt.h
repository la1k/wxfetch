/*
 *  wxdecode
 *  Copyright (c) 2016 Asgeir Bjorgan (LA9SSA)
 *  Copyright (c) 2004-2005 by Thierry Leconte (F4DWV)
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#ifndef DSP_H_DEFINED
#define DSP_H_DEFINED

#include "filter.h"

#define APT_IMG_WIDTH 2080
#define APT_SYNC_WIDTH 39
#define APT_SPC_WIDTH 47
#define APT_TELE_WIDTH 45
#define APT_CHANNEL_WIDTH 909
#define APT_CHANNEL_OFFSET (APT_SYNC_WIDTH + APT_SPC_WIDTH + APT_CHANNEL_WIDTH + APT_TELE_WIDTH)

#include "buffer.h"

typedef struct {
	double PhaseOsc;
	double FreqOsc;
	iirbuff_t Ifilterbuff, Qfilterbuff;
	float fr;
} pll_t;

#define BLKAMP 256
typedef struct {
	float ambuff[BLKAMP]; 
	int nam; //current number of samples in ambuff
	int idxam; //current start index in ambuff

	double last_max_correlation; //last maximum correlation
	pll_t phaselock_state; //state of the phaselock filter

	//demodulated pixels left over from the last run
	float leftover_pixels[APT_IMG_WIDTH];
	int num_leftover_pixels;
	
	//weird variables
	int synced;
	double FreqLine;
} apt_t;

/**
 * Decode an APT signal into a single row of pixels.
 * 
 * \param apt APT instance
 * \param sound_buffer Recorded signal buffer, assumed samplerate 11025 Hz
 * \param pixel_values Row of output pixel values, of length 2080
 * \return 1 if successful, 0 if not
 **/
int apt_decode(apt_t *apt, buffer_t *sound_buffer, float *pixel_values);

/**
 * Initialize internal variables for APT decoding.
 *
 * \param apt APT instance
 **/
void apt_initialize(apt_t *apt);

#include <opencv2/core/core.hpp>

/**
 * Get image corresponding to specified channel.
 * 
 * \param Raw APT image
 * \param channel Image channel index (0 or 1)
 * \return Image channel corresponding to the specified channel index
 **/
cv::Mat apt_image_channel(cv::Mat raw_image, int channel);

#endif
