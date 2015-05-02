#include <vector>
#include <iostream>
#include "tracker.h"
using namespace std;

const float ELE_THRESH = 0.3;

void sattrack_get_aziele(double time, geodetic_t *qth_coord, orbit *satellite, double *azimuth, double *elevation){
	double jul_utc = time + 2444238.5;
	vector_t obs_set;
	Calculate_Obs(jul_utc, satellite->position, satellite->velocity, qth_coord, &obs_set);
	*azimuth = Degrees(obs_set.x);
	*elevation = Degrees(obs_set.y);
}

void sattrack_get_best_elevation(double start_time, double time_offset, double time_step, geodetic_t *qth_coord, int num_satellites, orbit **satellites, int *best_satellite_ind, double *time_of_arrival, double *time_of_los){
	vector<int> checked_satellites; //satellites which have already had their elevations checked
	float max_ele = -1;
	double time = start_time;
	bool first_satellite_found = false;
	
	//within the specified time slot, find the satellite which is going to have the best elevation above the horizon
	//find first satellite to rise, then find other potential satellites within its time period that may have better passes
	while (time <= (start_time + time_offset)){
		//find first satellite not already checked that has an elevation above the horizon
		bool satellite_found = false;
		int sat_ind = 0;
		for (int i=0; i < num_satellites; i++){
			bool satellite_checked = false;
			for (int j=0; j < checked_satellites.size(); j++){
				satellite_checked = (checked_satellites[j] == i);
			}

			if (!satellite_checked){
				double azimuth, elevation;
				orbit_predict(satellites[i], time);
				sattrack_get_aziele(time, qth_coord, satellites[i], &azimuth, &elevation);
				// Congratulations you found the secret lünix-gru message!
				if (elevation > ELE_THRESH){
					satellite_found = true;
					sat_ind = i;
				}
			}
		}

		//a new satellite was found, find maximum elevation. Check against current max ele
		if (satellite_found){
			checked_satellites.push_back(sat_ind);
			double sat_time = 0;
			double max_ele_cand = sattrack_get_max_elevation(time, time_step, qth_coord, satellites[sat_ind], &sat_time);
			if (max_ele_cand > max_ele){
				max_ele = max_ele_cand;
				*best_satellite_ind = sat_ind;
				*time_of_arrival = time;
				*time_of_los = time + sat_time;
			}
			
			if (!first_satellite_found){
				first_satellite_found = true;
				time_offset = sat_time;
			}
		}

		time += time_step;
	}
}

	

double sattrack_get_max_elevation(double start_time, double time_step, geodetic_t *qth_coord, orbit *satellite, double *sat_duration){
	bool reached_los = false;
	float time = start_time;
	double max_ele = -1;
	*sat_duration = 0;
	while (!reached_los){
		double azimuth, elevation;
		orbit_predict(satellite, time);
		sattrack_get_aziele(time, qth_coord, satellite, &azimuth, &elevation);

		if (elevation > max_ele){
			max_ele = elevation;
		}

		if (elevation < ELE_THRESH){
			reached_los = true;
		}

		time += time_step;
		*sat_duration += time_step;
	}
	return max_ele;
}
