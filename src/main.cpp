#include <iostream>
#include <vector>
#include <sstream>
#include <cmath>
#include <ctime>
#include <cstring>
#include <unistd.h>
//#include "receiver.h"
#include "tracker.h"
using namespace std;

void wxsat_prepare(vector<predict_orbit_t*> *orbits, vector<float> *frequencies){
	//TODO: Read from file.
	char *tle_noaa15[2] = {"1 25338U 98030A   15111.44744815  .00000353  00000-0  16813-3 0  9990",
	"2 25338  98.7719 110.0498 0010142 339.5996  20.4777 14.25603267880750"};
	char *tle_noaa18[2] = {"1 28654U 05018A   15111.52351205  .00000264  00000-0  16910-3 0  9993",
	"2 28654  99.1819 102.7929 0015245 117.6325 242.6395 14.12181499511071"};
	char *tle_noaa19[2] = {"1 33591U 09005A   15111.47492914  .00000377  00000-0  23086-3 0  9993",
	"2 33591  98.9850  61.2335 0013646 334.4839  25.5657 14.11916313319481"};

	orbits->push_back(predict_create_orbit(predict_parse_tle(tle_noaa15)));
	strcpy((*orbits)[0]->name, "NOAA15");
	orbits->push_back(predict_create_orbit(predict_parse_tle(tle_noaa18)));
	strcpy((*orbits)[1]->name, "NOAA18");
	orbits->push_back(predict_create_orbit(predict_parse_tle(tle_noaa19)));
	strcpy((*orbits)[2]->name, "NOAA19");

	frequencies->push_back(137.62*1.0e03);
	frequencies->push_back(137.9125*1.0e03);
	frequencies->push_back(137.1000*1.0e03);
}

int main(){
	//satellites
	vector<predict_orbit_t*> orbits;
	vector<float> frequencies;
	wxsat_prepare(&orbits, &frequencies);

	//QTH
	predict_observer_t *observer = predict_create_observer("test_qth", 63.422429*M_PI/180.0, -10.395282*M_PI/180.0, 0);

	while (true) {
		predict_julian_date_t curr_time = predict_to_julian(time(NULL));

		vector<predict_julian_date_t> orbits_aos;
		vector<predict_julian_date_t> orbits_los;
		vector<int> above_horizon;
		vector<float> orbits_max_ele;
		for (int i=0; i < orbits.size(); i++){
			//calculate AOS/LOS
			predict_orbit_t *orbit = orbits[i];
			predict_julian_date_t aos = predict_next_aos(observer, orbit, curr_time);
			predict_julian_date_t los = predict_next_los(observer, orbit, curr_time);

			//check whether orbit is already above the horizon
			predict_orbit(orbits[i], curr_time);
			struct predict_observation obs;
			predict_observe_orbit(observer, orbits[i], &obs);
			if (obs.elevation > 0){
				aos = curr_time;
				above_horizon.push_back(i);
			}

			//calculate approximate max elevation for the pass
			int num_steps = 100;
			predict_julian_date_t pred_time = aos;
			double max_ele = -100;
			for (int j=0; j < num_steps; j++){
				predict_orbit(orbit, pred_time);
				struct predict_observation obs;
				predict_observe_orbit(observer, orbit, &obs);
				if (obs.elevation > max_ele){
					max_ele = obs.elevation;
				}
				pred_time = (los - aos)/(1.0*num_steps)*j + aos;
			}
			orbits_max_ele.push_back(max_ele);
			orbits_los.push_back(los);
			orbits_aos.push_back(aos);
		}

		//find first orbit to track
		predict_julian_date_t next_los;
		predict_julian_date_t next_aos;
		predict_orbit_t *orbit = NULL;
		double waiting_time = 0;
		if (above_horizon.size() > 0){
			double max_ele = -10;
			next_aos = curr_time;

			//find orbit that will have the best pass
			for (int i=0; i < above_horizon.size(); i++){
				if (orbits_max_ele[above_horizon[i]] > max_ele){
					max_ele = orbits_max_ele[above_horizon[i]];
					orbit = orbits[above_horizon[i]];
					next_los = orbits_los[above_horizon[i]];
				}
			}
		} else {
			//FIXME: Check overlapping orbits, find an orbit sequence that maximizes elevation, duration, ...
			//find first orbit to rise above the horizon
			predict_julian_date_t first_aos = orbits_aos[0];
			for (int i=0; i < orbits_aos.size(); i++){
				if (orbits_aos[i] < first_aos){
					first_aos = orbits_aos[i];
					next_los = orbits_los[i];
					orbit = orbits[i];
				}
			}
			next_aos = first_aos;
		}

		//wait until pass is here
		const int MAX_NUM_CHARS = 512;
		char time_string[MAX_NUM_CHARS];
		time_t aos_epoch = predict_from_julian(next_aos);
		strftime(time_string, MAX_NUM_CHARS, "%j.%H:%M:%S", localtime(&aos_epoch));
		double seconds = predict_from_julian(next_aos) - time(NULL);
		waiting_time = difftime(predict_from_julian(curr_time), predict_from_julian(next_aos));

		cout << "Waiting until next pass: " << orbit->name << " at " << time_string << ", in " << seconds << " seconds." << endl;
		sleep(seconds);

		curr_time = next_aos;
		while (curr_time < next_los) {
			curr_time = predict_to_julian(time(NULL));
			predict_orbit(orbit, curr_time);
			struct predict_observation obs;
			predict_observe_orbit(observer, orbit, &obs);
			cout << obs.azimuth*180.0/M_PI << " " << obs.elevation*180.0/M_PI << endl;
			sleep(1);
		}
		cin.get();




	}


#if 0
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
#endif
}
