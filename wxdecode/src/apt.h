#ifndef DSP_H_DEFINED
#define DSP_H_DEFINED

#include "filter.h"
#include "filtercoeff.h"

#define BLKAMP 256
#define PixelLine 2080


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
	int npv;
	int synced;
	double max;
	double FreqLine;
	
	float pixels[PixelLine + SyncFilterLen];

	pll_t phaselock;
} apt_t;


/**
 * Decode an APT signal into a pixel array.
 * 
 * \param sound_buffer Recorded signal buffer
 * \param apt APT instance
 * \param pixel_values Output pixel values
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
