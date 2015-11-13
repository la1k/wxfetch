#ifndef DSP_H_DEFINED
#define DSP_H_DEFINED

#include "filter.h"
#include "filtercoeff.h"

#define BLKAMP 256
#define PixelLine 2080

typedef struct {
	double PhaseOsc;
	double FreqOsc;
	iirbuff_t Ifilterbuff, Qfilterbuff;
	float fr;
} pll_t;

typedef struct {
	float ambuff[BLKAMP];
	int nam;
	int idxam;
	float pixels[PixelLine + SyncFilterLen];
	int npv;
	int synced;
	double max;
	double FreqLine;

	pll_t phaselock;
} apt_t;

int getpixelrow(apt_t *apt, float *pixelv);
void apt_initialize(apt_t *apt);

#endif
