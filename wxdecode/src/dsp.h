#ifndef DSP_H_DEFINED
#define DSP_H_DEFINED

#include "filter.h"
#include "filtercoeff.h"

#define BLKAMP 256
#define PixelLine 2080

#define Fe 11025.0
#define Fc 2400.0
#define DFc 50.0
#define Fp (2*PixelLine)
#define RSMULT 10
#define Fi (Fp*RSMULT)

/* pll coeff */
#define K1 5e-3
#define K2 3e-6
#define BLKIN 1024

typedef struct {
	double PhaseOsc;
	double FreqOsc;
	iirbuff_t Ifilterbuff, Qfilterbuff;
	float fr;
} pll_t;

typedef struct {
	float ambuff[BLKAMP]; //amplitude buffer, containing demodulated values
	int nam; //current number of samples in amplitude buffer
	int idxam; //current start index in the amplitude buffer
	int npv; //current number of pixels in the pixel buffer
	int synced;
	double max;
	double FreqLine;
	
	float pixels[PixelLine + SyncFilterLen];

	pll_t phaselock;
} apt_t;

typedef struct {
	float *data;
	int total_num_samples;
	int current_start_position;
	int current_end_position;
	int current_num_samples;
} buffer_t;

void buffer_initialize(buffer_t *buffer, int tot_num_samples);
int buffer_fill(buffer_t *buffer, int num_samples, float *samples);
int buffer_read(buffer_t *buffer, int num_samples, float *samples);
int buffer_length(buffer_t *buffer);

int getpixelrow(apt_t *apt, float *pixelv);
void apt_initialize(apt_t *apt);

#endif
