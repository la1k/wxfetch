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

#define BLKAMP 256
#define NUM_PIXELS_IN_ROW 2080

#include "buffer.h"

typedef struct {
	double PhaseOsc;
	double FreqOsc;
	iirbuff_t Ifilterbuff, Qfilterbuff;
	float fr;
} pll_t;

typedef struct {
	float ambuff[BLKAMP]; 
	int nam; //current number of samples in ambuff
	int idxam; //current start index in ambuff

	double last_max_correlation; //last maximum correlation
	pll_t phaselock_state; //state of the phaselock filter

	//demodulated pixels left over from the last run
	float leftover_pixels[NUM_PIXELS_IN_ROW];
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

#endif
