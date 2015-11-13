/*
 *  Atpdec
 *  Copyright (c) 2004-2005 by Thierry Leconte (F4DWV)
 *
 *      $Id: main.c,v 1.15 2005/05/20 23:01:24 f4dwv Exp $
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

#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include "w32util.h"
#else
#include <libgen.h>
#endif
#include <string.h>
#include <sndfile.h>


extern int getpixelrow(float *pixelv);

#define SYNC_WIDTH 39
#define SPC_WIDTH 47
#define TELE_WIDTH 45
#define CH_WIDTH  909
#define CH_OFFSET  (SYNC_WIDTH+SPC_WIDTH+CH_WIDTH+TELE_WIDTH)
#define IMG_WIDTH  2080

static SNDFILE *inwav;

static int initsnd(char *filename)
{
    SF_INFO infwav;

/* open wav input file */
    infwav.format = 0;
    inwav = sf_open(filename, SFM_READ, &infwav);
    if (inwav == NULL) {
	fprintf(stderr, "could not open %s\n", filename);
	return (1);
    }
    if (infwav.samplerate != 11025) {
	fprintf(stderr, "Bad Input File sample rate: %d. Must be 11025\n",
		infwav.samplerate);
	return (1);
    }
    if (infwav.channels != 1) {
	fprintf(stderr, "Too many channels in input file : %d\n",
		infwav.channels);
	return (1);
    }

    return (0);
}

int getsample(float *sample, int nb)
{
    return (sf_read_float(inwav, sample, nb));
}

int main(int argc, char **argv)
{
    float *prow[3000];
    int nrow;

    for (nrow = 0; nrow < 3000; nrow++)
	prow[nrow] = NULL;

/* open snd input */
	if (initsnd(argv[1]))
	    exit(1);

/* main loop */
	for (nrow = 0; nrow < 3000; nrow++) {
	    if (prow[nrow] == NULL) {
		prow[nrow] = (float *) malloc(sizeof(float) * 2150);
		for (int i=0; i < 2150; i++){
			prow[nrow][i] = 0;
		}
	    }
	    if (getpixelrow(prow[nrow]) == 0)
		break;
	    for (int i=0; i < 2150; i++){
		    printf("%f ", prow[nrow][i]);
	    }
	    printf("\n");
	}
	sf_close(inwav);

}
