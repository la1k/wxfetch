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

#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include "w32util.h"
#else
#include <libgen.h>
#endif
#include <string.h>
#include <sndfile.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "apt.h"
#include "buffer.h"
#include <iostream>
using namespace std;

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
	if (initsnd(argv[1]))
		exit(1);

	int num_cols = 2150;

	cv::Mat img(0, num_cols, CV_32FC1);

	apt_t apt;
	apt_initialize(&apt);

	buffer_t sound_buffer;
	buffer_initialize(&sound_buffer, 11025);
	float sound_temp[11025];

	bool finished = false;
	float *pixel_data = new float[num_cols]();
	int max_consumed_samples = -1;
	while (!finished) {
		//fill buffer with sound data
		int num_samples = buffer_capacity(&sound_buffer);
		int read_samples = getsample(sound_temp, num_samples);
		if (read_samples <= 0) {
			break;
		}
		buffer_fill(&sound_buffer, read_samples, sound_temp);
		int length = buffer_length(&sound_buffer);

		//decode APT signal
		int retval = apt_decode(&apt, &sound_buffer, pixel_data);
		if (retval != 0) {
			img.push_back(cv::Mat(1, num_cols, CV_32FC1, pixel_data).clone());
		}
		int consumed_samples = length - buffer_length(&sound_buffer);
		if (consumed_samples > max_consumed_samples){
			max_consumed_samples = consumed_samples;
			cout << "Consumed samples: " << consumed_samples << endl;
		}
	}
	sf_close(inwav);

	delete [] pixel_data;
	cv::imshow("test", img/255);
	cv::waitKey();

	imwrite("test.png", img);
}
