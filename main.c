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
#ifdef WIN32
#include "w32util.h"
#else
#include <libgen.h>
#endif
#include <string.h>
#include <sndfile.h>
#include <png.h>

#include "version.h"

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
    char pngfilename[1024];
    char name[1024];
    char pngdirname[1024] = "";
    char imgopt[20] = "ac";
    float *prow[3000];
    char *chid[6] = { "1", "2", "3A", "4", "5", "3B" };
    int nrow;
    int ch;
    int c;

    printf("%s\n", version);

    opterr = 0;
    while ((c = getopt(argc, argv, "c:d:i:s:")) != EOF) {
	switch (c) {
	case 'd':
	    strcpy(pngdirname, optarg);
	    break;
	case 'c':
	    readfconf(optarg);
	    break;
	case 'i':
	    strcpy(imgopt, optarg);
	    break;
	case 's':
	    satnum = atoi(optarg)-15;
	    if (satnum < 0 || satnum > 3) {
		fprintf(stderr, "invalid satellite number  : must be in [15-18]\n");
		exit(1);
	    }
	    break;
	default:
	    usage();
	}
    }

    for (nrow = 0; nrow < 3000; nrow++)
	prow[nrow] = NULL;

    for (; optind < argc; optind++) {
	int a = 0, b = 0;

	strcpy(pngfilename, argv[optind]);
	strcpy(name, basename(pngfilename));
	strtok(name, ".");
	if (pngdirname[0] == '\0') {
	    strcpy(pngfilename, argv[optind]);
	    strcpy(pngdirname, dirname(pngfilename));
	}

/* open snd input */
	if (initsnd(argv[optind]))
	    exit(1);

/* main loop */
	printf("Decoding: %s \n", argv[optind]);
	for (nrow = 0; nrow < 3000; nrow++) {
	    if (prow[nrow] == NULL)
		prow[nrow] = (float *) malloc(sizeof(float) * 2150);
	    if (getpixelrow(prow[nrow]) == 0)
		break;
	    printf("%d\r", nrow);
	    fflush(stdout);
	}
	printf("\nDone\n");
	sf_close(inwav);

/* raw image */
	if (strchr(imgopt, (int) 'r') != NULL) {
	    sprintf(pngfilename, "%s/%s-r.png", pngdirname, name);
	    ImageOut(pngfilename, "raw", prow, nrow, IMG_WIDTH, 0);
	}

/* Channel A */
	if (((strchr(imgopt, (int) 'a') != NULL)
	     || (strchr(imgopt, (int) 'c') != NULL)
	     || (strchr(imgopt, (int) 'd') != NULL))) {
	    ch = Calibrate(prow, nrow, SYNC_WIDTH);
	    if (ch >= 0) {
		if (strchr(imgopt, (int) 'a') != NULL) {
		    sprintf(pngfilename, "%s/%s-%s.png", pngdirname, name,
			    chid[ch]);
		    ImageOut(pngfilename, chid[ch], prow, nrow, 
			     SPC_WIDTH + CH_WIDTH + TELE_WIDTH,
			     SYNC_WIDTH);
		}
	    }
	    if (ch < 2)
		a = 1;
	}

/* Channel B */
	if ((strchr(imgopt, (int) 'b') != NULL)
	    || (strchr(imgopt, (int) 'c') != NULL)
	    || (strchr(imgopt, (int) 't') != NULL)
	    || (strchr(imgopt, (int) 'd') != NULL)) {
	    ch = Calibrate(prow, nrow, CH_OFFSET + SYNC_WIDTH);
	    if (ch >= 0) {
		if (strchr(imgopt, (int) 'b') != NULL) {
		    sprintf(pngfilename, "%s/%s-%s.png", pngdirname, name,
			    chid[ch]);
		    ImageOut(pngfilename, chid[ch], prow, nrow, 
			     SPC_WIDTH + CH_WIDTH + TELE_WIDTH,
			     CH_OFFSET + SYNC_WIDTH);
		}
	    }
	    if (ch > 2) {
		b = 1;
		Temperature(prow, nrow, ch, CH_OFFSET + SYNC_WIDTH);
		if (strchr(imgopt, (int) 't') != NULL) {
		    sprintf(pngfilename, "%s/%s-t.png", pngdirname, name);
		    ImageOut(pngfilename, "Temperature", prow, nrow,
			     CH_WIDTH, CH_OFFSET + SYNC_WIDTH + SPC_WIDTH);
		}
	    }
	}

/* distribution */
	if (a && b && strchr(imgopt, (int) 'd') != NULL) {
	    sprintf(pngfilename, "%s/%s-d.pnm", pngdirname, name);
	}

/* color image */
	if (a && b && strchr(imgopt, (int) 'c') != NULL) {
	    sprintf(pngfilename, "%s/%s-c.png", pngdirname, name);
	    ImageColorOut(pngfilename, prow, nrow);
	}

    }
    exit(0);
}
