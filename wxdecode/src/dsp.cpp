/*
 *  Atpdec 
 *  Copyright (c) 2004 by Thierry Leconte (F4DWV)
 *
 *      $Id: dsp.c,v 1.12 2004/04/24 07:19:48 f4dwv Exp $
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
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846	/* for OS that don't know it */
#endif				/*  */
#include "filter.h"
#include "filtercoeff.h"
#include <stdlib.h>


#include "dsp.h"

extern int getsample(float *inbuff, int nb);



float pll_initialize(pll_t *pll_m){
	pll_m->PhaseOsc = 0.0;
	pll_m->FreqOsc = Fc / Fe;
	pll_m->fr = Fc / Fe;
	pll_m->FreqOsc = Fc / Fe;
}

static float pll(pll_t *pll_m, float In)
{
    double Io, Qo;
    double Ip, Qp;
    double DPhi;
    double DF;

    /* quadrature oscillator */
    Io = cos(pll_m->PhaseOsc);
    Qo = sin(pll_m->PhaseOsc);

    /* phase detector */
    Ip = iir(In * Io, &(pll_m->Ifilterbuff), &PhaseFilterCf);
    Qp = iir(In * Qo, &(pll_m->Qfilterbuff), &PhaseFilterCf);
    DPhi = -atan2(Qp, Ip) / M_PI;

    /*  loop filter  */
    DF = K1 * DPhi + pll_m->FreqOsc;
    pll_m->FreqOsc += K2 * DPhi;
    if (pll_m->FreqOsc > ((Fc + DFc) / Fe))
	pll_m->FreqOsc = (Fc + DFc) / Fe;
    if (pll_m->FreqOsc < ((Fc - DFc) / Fe))
	pll_m->FreqOsc = (Fc - DFc) / Fe;
    pll_m->PhaseOsc += 2.0 * M_PI * DF;

    if (pll_m->PhaseOsc > M_PI)
	pll_m->PhaseOsc -= 2.0 * M_PI;
    if (pll_m->PhaseOsc <= -M_PI)
	pll_m->PhaseOsc += 2.0 * M_PI;
	
    pll_m->fr = 0.25 * pll_m->FreqOsc + 0.75 * pll_m->fr;
    return (float) (In * Io);
}

static double offset = 0.0;

void getamp(pll_t *phaselock, float *ambuff, float *sound_samples, int nb)
{
    int n;
    for (n = 0; n < nb; n++) {
	ambuff[n] = pll(phaselock, sound_samples[n]);
    }
}


void apt_initialize(apt_t *apt)
{
	apt->npv = 0;
	apt->synced = 0;
	apt->max = 0.0;
	apt->nam = 0;
	apt->idxam = 0;
	apt->FreqLine = 1.0;

	pll_initialize(&apt->phaselock);
}

int getpixelv(apt_t *apt, float *pvbuff, int nb)
{
    int n;

    for (n = 0; n < nb; n++) {
	double mult;
	int shift;

	if (apt->nam < BLKAMP) {
	    int res;
	    memmove(apt->ambuff, &(apt->ambuff[apt->idxam]), apt->nam * sizeof(float));
	    apt->idxam = 0;

	    float sound_buff[BLKAMP];
	    int num_samples = BLKAMP - apt->nam;
	    res = getsample(sound_buff, num_samples);
	    getamp(&(apt->phaselock), &(apt->ambuff[apt->nam]), sound_buff, BLKAMP - apt->nam);
	    apt->nam += res;
	    if (apt->nam < BLKAMP)
		return (n);
	}

	mult = (double) Fi * apt->phaselock.fr / Fc * apt->FreqLine;

	pvbuff[n] =
	    rsfir(&(apt->ambuff[apt->idxam]), rsfilter, RSFilterLen, offset,
		  mult) * mult * 2 * 256.0;

	shift = (int) ((RSMULT - offset + mult - 1) / mult);
	offset = shift * mult + offset - RSMULT;
	apt->idxam += shift;
	apt->nam -= shift;
    }
    return (nb);
}


int getpixelrow(apt_t *apt, float *pixelv)
{
    double corr, ecorr, lcorr;
    int res;

    if (apt->npv > 0)
	memmove(pixelv, apt->pixels, apt->npv * sizeof(float));
    if (apt->npv < SyncFilterLen + 2) {
	res = getpixelv(apt, &(pixelv[apt->npv]), SyncFilterLen + 2 - apt->npv);
	apt->npv += res;
	if (apt->npv < SyncFilterLen + 2)
	    return (0);
    }

    /* test sync */
    corr = fir(&(pixelv[1]), Sync, SyncFilterLen);
    ecorr = fir(pixelv, Sync, SyncFilterLen);
    lcorr = fir(&(pixelv[2]), Sync, SyncFilterLen);
    apt->FreqLine = 1.0 + (ecorr - lcorr) / corr / PixelLine / 4.0;
    if (corr < 0.75 * apt->max) {
	apt->synced = 0;
	apt->FreqLine = 1.0;
    }
    apt->max = corr;
    if (apt->synced < 8) {
	int shift, mshift;

	if (apt->npv < PixelLine + SyncFilterLen) {
	    res =
		getpixelv(apt, &(pixelv[apt->npv]), PixelLine + SyncFilterLen - apt->npv);
	    apt->npv += res;
	    if (apt->npv < PixelLine + SyncFilterLen)
		return (0);
	}

	/* lookup sync start */
	mshift = 0;
	for (shift = 1; shift < PixelLine; shift++) {
	    double corr;

	    corr = fir(&(pixelv[shift + 1]), Sync, SyncFilterLen);
	    if (corr > apt->max) {
		mshift = shift;
		apt->max = corr;
	    }
	}
	if (mshift != 0) {
	    memmove(pixelv, &(pixelv[mshift]),
		    (apt->npv - mshift) * sizeof(float));
	    apt->npv -= mshift;
	    apt->synced = 0;
	    apt->FreqLine = 1.0;
	} else
	    apt->synced += 1;
    }
    if (apt->npv < PixelLine) {
	res = getpixelv(apt, &(pixelv[apt->npv]), PixelLine - apt->npv);
	apt->npv += res;
	if (apt->npv < PixelLine)
	    return (0);
    }
    if (apt->npv == PixelLine) {
	apt->npv = 0;
    } else {
	memmove(apt->pixels, &(pixelv[PixelLine]),
		(apt->npv - PixelLine) * sizeof(float));
	apt->npv -= PixelLine;
    }

    return (1);
}

void buffer_initialize(buffer_t *buffer, int tot_num_samples)
{
	buffer->data = (float*)malloc(sizeof(float)*tot_num_samples);
	buffer->total_num_samples = tot_num_samples;
	buffer->current_start_position = 0;
	buffer->current_num_samples = 0;
}

#include <iostream>
using namespace std;
int buffer_fill(buffer_t *buffer, int num_samples, float *samples)
{
	//number of samples we can write to the buffer
	int capacity = buffer->total_num_samples - buffer->current_num_samples;
	int write_samples = num_samples;
	if (num_samples > capacity) {
		write_samples = capacity;
	}

	int first_part_len = write_samples;
	int second_part_len = 0;
	int end_space = buffer->total_num_samples - (buffer->current_start_position + buffer->current_num_samples);

	if (end_space <= 0) { //no space left at the end of the array
		first_part_len = 0;
		second_part_len = write_samples;
	} else if (end_space < first_part_len) { //some space at the end, some space at the beginning
		first_part_len = end_space;
		second_part_len = write_samples - end_space;
		end_space = 0;
	}

	if (first_part_len > 0) {
		memcpy(buffer->data + buffer->current_start_position + buffer->current_num_samples, samples, sizeof(float)*first_part_len);
	}

	if (second_part_len > 0) {
		int start_copy = abs(end_space);
		memcpy(buffer->data + start_copy, samples + first_part_len, sizeof(float)*second_part_len);
	}

	buffer->current_num_samples += write_samples;
	return write_samples;
}

int buffer_read(buffer_t *buffer, int num_samples, float *samples)
{
	if (buffer->current_num_samples <= 0) {
		return 0;
	}

	int read_samples = num_samples;
	if (num_samples > buffer->current_num_samples) {
		read_samples = buffer->current_num_samples;
	}

	int first_part_len = read_samples;
	int second_part_len = 0;
	if ((buffer->current_start_position + read_samples) > buffer->total_num_samples) {
		first_part_len = buffer->total_num_samples - buffer->current_start_position;
		second_part_len = read_samples - first_part_len;
	}
	
	if (first_part_len > 0) {
		memcpy(samples, buffer->data + buffer->current_start_position, sizeof(float)*first_part_len);
		buffer->current_num_samples -= first_part_len;
		buffer->current_start_position += first_part_len % buffer->total_num_samples;
	}

	if (second_part_len > 0) {
		memcpy(samples + first_part_len, buffer->data, sizeof(float)*second_part_len);
		buffer->current_num_samples -= second_part_len;
		buffer->current_start_position = second_part_len;
	}
	return read_samples;
}
