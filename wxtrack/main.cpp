#include <iostream>
#include <vector>
#include <sstream>
#include <cmath>
#include <ctime>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
//#include "receiver.h"
#include <predict/predict.h>
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
	int last_elevation = 0;
	int last_azimuth = 0;

	//clear screen
	printf("\e[1;1H\e[2J");

	while (true) {
		predict_julian_date_t curr_time = predict_to_julian(time(NULL));

		vector<struct predict_observation> observations;

		double max_elevation = -2*M_PI;
		int orbit_ind_max = 0;

		//find orbit with highest current elevation
		for (int i=0; i < orbits.size(); i++) {
			predict_orbit(orbits[i], curr_time);
			struct predict_observation obs;
			predict_observe_orbit(observer, orbits[i], &obs);

			if (obs.elevation > max_elevation) {
				max_elevation = obs.elevation;
				orbit_ind_max = i;
			}

			observations.push_back(obs);
		}

		//track orbit
		int azimuth = observations[orbit_ind_max].azimuth*180.0/M_PI;
		int elevation = observations[orbit_ind_max].elevation*180.0/M_PI;

		//print information
		printf("\033[0;0HOrbit with current highest elevation: %s, aziele %f, %f ", orbits[orbit_ind_max]->name, observations[orbit_ind_max].azimuth*180.0/M_PI, observations[orbit_ind_max].elevation*180.0/M_PI);
		if (elevation < 0) {
			printf("(not tracked)\n");
		} else {
			printf("(tracked)\n");
		}
		printf("\nOther orbits:\n");
		for (int i=0; i < orbits.size(); i++) {
			if (i != orbit_ind_max) {
				printf("%s: %f, %f\n", orbits[i]->name, observations[i].azimuth*180.0/M_PI, observations[i].elevation*180.0/M_PI);
			}
		}
		fflush(stdout);
		sleep(1);
	}
}
