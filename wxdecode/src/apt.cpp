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
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846	/* for OS that don't know it */
#endif				/*  */
#include "apt.h"
#include "filter.h"
#include "filtercoeff.h"
#include "buffer.h"

#define Fe 11025.0
#define Fc 2400.0
#define DFc 50.0
#define Fp (2*NUM_PIXELS_IN_ROW)
#define RSMULT 10
#define Fi (Fp*RSMULT)
#include <stdio.h>

/* pll coeff */
#define K1 5e-3
#define K2 3e-6
#define BLKIN 1024



float pll_initialize(pll_t *pll_m){
	pll_m->PhaseOsc = 0.0;
	pll_m->FreqOsc = Fc / Fe;
	pll_m->fr = Fc / Fe;
	pll_m->FreqOsc = Fc / Fe;

	iirbuff_initialize(&(pll_m->Ifilterbuff));
	iirbuff_initialize(&(pll_m->Qfilterbuff));
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
	apt->num_leftover_pixels = 0;
	apt->synced = 0;
	apt->last_max_correlation = 0.0;
	apt->nam = 0;
	apt->idxam = 0;
	apt->FreqLine = 1.0;

	pll_initialize(&apt->phaselock_state);
}

int getpixelv(buffer_t *sound_buffer, apt_t *apt, float *pvbuff, int nb)
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
	    res = buffer_read(sound_buffer, num_samples, sound_buff);
	    getamp(&(apt->phaselock_state), &(apt->ambuff[apt->nam]), sound_buff, res);
	    apt->nam += res;
	    if (apt->nam < BLKAMP)
		return (n);
	}

	mult = (double) Fi * apt->phaselock_state.fr / Fc * apt->FreqLine;

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


int apt_decode(apt_t *apt, buffer_t *sound_buffer, float *pixels_out)
{
    double corr, ecorr, lcorr;
    float pixelv[NUM_PIXELS_IN_ROW + SyncFilterLen] = {};
    int res;
    int npv = apt->num_leftover_pixels;
    if (apt->num_leftover_pixels > 0) {
	memmove(pixelv, apt->leftover_pixels, npv * sizeof(float));
	apt->num_leftover_pixels = 0;
    }
    if (npv < SyncFilterLen + 2) {
	res = getpixelv(sound_buffer, apt, &(pixelv[npv]), SyncFilterLen + 2 - npv);
	npv += res;
	if (npv < SyncFilterLen + 2)
	    return (0);
    }

    /* test sync */
    corr = fir(&(pixelv[1]), Sync, SyncFilterLen);
    ecorr = fir(pixelv, Sync, SyncFilterLen);
    lcorr = fir(&(pixelv[2]), Sync, SyncFilterLen);
    apt->FreqLine = 1.0 + (ecorr - lcorr) / corr / NUM_PIXELS_IN_ROW / 4.0;
    if (corr <= 0.75 * apt->last_max_correlation) {
	apt->synced = 0;
	apt->FreqLine = 1.0;
    }
    apt->last_max_correlation = corr;
    if (apt->synced < 8) {
	int shift, mshift;

	if (npv < NUM_PIXELS_IN_ROW + SyncFilterLen) {
	    res =
		getpixelv(sound_buffer, apt, &(pixelv[npv]), NUM_PIXELS_IN_ROW + SyncFilterLen - npv);
	    npv += res;
	    if (npv < NUM_PIXELS_IN_ROW + SyncFilterLen)
		return (0);
	}

	/* lookup sync start */
	mshift = 0;
	for (shift = 1; shift < NUM_PIXELS_IN_ROW; shift++) {
	    double corr;

	    corr = fir(&(pixelv[shift + 1]), Sync, SyncFilterLen);
	    if (corr > apt->last_max_correlation) {
		mshift = shift;
		apt->last_max_correlation = corr;
	    }
	}
	if (mshift != 0) {
	    memmove(pixelv, &(pixelv[mshift]),
		    (npv - mshift) * sizeof(float));
	    npv -= mshift;
	    apt->synced = 0;
	    apt->FreqLine = 1.0;
	} else
	    apt->synced += 1;
    }
    if (npv < NUM_PIXELS_IN_ROW) {
	res = getpixelv(sound_buffer, apt, &(pixelv[npv]), NUM_PIXELS_IN_ROW - npv);
	npv += res;
	if (npv < NUM_PIXELS_IN_ROW)
	    return (0);
    }
    if (npv == NUM_PIXELS_IN_ROW) {
	npv = 0;
    } else {
	memmove(apt->leftover_pixels, &(pixelv[NUM_PIXELS_IN_ROW]),
		(npv - NUM_PIXELS_IN_ROW) * sizeof(float));
	apt->num_leftover_pixels = npv - NUM_PIXELS_IN_ROW;
    }
    
    memcpy(pixels_out, pixelv, NUM_PIXELS_IN_ROW*sizeof(float));

    return (1);
}
