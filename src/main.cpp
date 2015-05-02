#include <iostream>
#include <vector>
#include <sstream>
#include "receiver.h"
#include "tracker.h"
using namespace std;

void wxsat_prepare(int *num_satellites, struct orbit*** satellites, float **freqs){
	const char *tle_noaa15[2] = {"1 25338U 98030A   15111.44744815  .00000353  00000-0  16813-3 0  9990",
	"2 25338  98.7719 110.0498 0010142 339.5996  20.4777 14.25603267880750"};
	const char *tle_noaa18[2] = {"1 28654U 05018A   15111.52351205  .00000264  00000-0  16910-3 0  9993",
	"2 28654  99.1819 102.7929 0015245 117.6325 242.6395 14.12181499511071"};
	const char *tle_noaa19[2] = {"1 33591U 09005A   15111.47492914  .00000377  00000-0  23086-3 0  9993",
	"2 33591  98.9850  61.2335 0013646 334.4839  25.5657 14.11916313319481"};

	*num_satellites = 1;
	*satellites = new struct orbit*[*num_satellites];
	(*satellites)[0] = new struct orbit;
	(*satellites)[1] = new struct orbit;
	(*satellites)[2] = new struct orbit;
	orbit_init((*satellites)[0], tle_noaa15);
	orbit_init((*satellites)[1], tle_noaa18);
	orbit_init((*satellites)[2], tle_noaa19);

	*freqs = new float[*num_satellites];
	(*freqs)[0] = 137.62*1.0e03;
	(*freqs)[1] = 137.9125*1.0e03;
	(*freqs)[2] = 137.1000*1.0e03;
}

int main(){
	//satellites
	int num_satellites = 0;
	struct orbit **satellites;
	float *freqs;
	wxsat_prepare(&num_satellites, &satellites, &freqs);
	
	//QTH
	geodetic_t obs_geodetic;
	obs_geodetic.lat = 63.422429*deg2rad;
	obs_geodetic.lon = -10.395282*deg2rad;
	obs_geodetic.alt = 25.0/1000.0;
	obs_geodetic.theta = 0;

	//RTL-SDR receiver
	receiver_t receiver;
	receiver_initialize(&receiver);


	double satcalc_time_step = 1.0f/(24.0f*60);
	double curr_time = 0;
	const double DAYS_TO_SECONDS_FACTOR = 24*60*60;

	while (true){
		//estimate start and stop time of next satellite pass
		curr_time = CurrentDaynum();
		double time_of_arrival = 0;
		int sat_ind = 0;
		double time_of_departure = 0;
		sattrack_get_best_elevation(curr_time, 1, satcalc_time_step, &obs_geodetic, num_satellites, satellites, &sat_ind, &time_of_arrival, &time_of_departure);

		double time_until_start_sec = (time_of_arrival - curr_time)*DAYS_TO_SECONDS_FACTOR;
		double duration_secs = (time_of_departure - time_of_arrival)*DAYS_TO_SECONDS_FACTOR;
		fprintf(stderr, "Calculated next satellite pass: cat %d in %f minutes, lasting %f minutes.\n", satellites[sat_ind]->catnum, time_until_start_sec/(60), duration_secs/60);

		//sleep until approx. start
		sleep(time_until_start_sec);

		//satellite is here! Start gnuradio and doppler shift calculations. 
		ostringstream filename;
		time_t curr_epoch = time(NULL);
		tm curr_datetime = *localtime(&curr_epoch);

		const int MAX_DATESTR_SIZE = 200;
		char datestr[MAX_DATESTR_SIZE];
		strftime(datestr, MAX_DATESTR_SIZE, "%Y-%m-%d-%H%M%S", &curr_datetime);

		filename << "wxfetch_cat_" << satellites[sat_ind]->catnum << "_" << datestr << ".wav";
			
		double sattrack_timestep = 1.0f;
		double sat_freqv = freqs[sat_ind];
		receiver_set_frequency(&receiver, sat_freqv);
		receiver_start(&receiver);
		receiver_set_filename(&receiver, filename.str().c_str());
		while (curr_time < time_of_departure){
			curr_time = CurrentDaynum();
			orbit_predict(satellites[sat_ind], curr_time);
	
			//FIXME: Doppler shift calculation. 
			double corr_freqv = sat_freqv;

			receiver_set_frequency(&receiver, corr_freqv);
			sleep(sattrack_timestep);
		}
		receiver_stop(&receiver);
		
		//sleep some extra time just to shake off the last satellite. 
		sleep(1);
	}
	
	return 0;
}
