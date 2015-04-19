const float ELE_THRESH = 0.3;

void sattrack_get_best_elevation(double start_time, double time_offset, double time_step, const geodetic_t *qth_coord, int num_satellites, const orbit_t **satellites, orbit_t *best_satellite, double *time_of_arrival){
	vector<int> checked_satellites; //satellites which have already had their elevations checked
	float max_ele = -1;
	double time = start_time;
	
	//within the specified time slot, find the satellite which is going to have the best elevation above the horizon
	while (time <= (start_time + time_offset)){
		//find first satellite not already checked that has an elevation above the horizon
		bool satellite_found = false;
		int sat_ind = 0;
		for (int i=0; i < num_satellite; i++){
			bool satellite_checked = false;
			for (int j=0; j < checked_satellites.size(); j++){
				satellite_checked = (checked_satellites[j] == i);
			}

			if (!satellite_checked){
				double azimuth, elevation;
				orbit_predict(satellites[i], time);
				sattrack_get_aziele(time, qth_coord, satellites[i], &azimuth, &elevation);

				if (elevation > ELE_THRESH){
					satellite_found = true;
					sat_ind = i;
				}
			}
		}

		//a new satellite was found, find maximum elevation. Check against current max ele
		if (satellite_found){
			satellite_checked.push_back(sat_ind);
			double max_ele_cand = sattrack_get_max_elevation(time, time_step, qth_coord, satellites[sat_ind]);
			if (max_ele_cand > max_ele){
				max_ele = max_ele_cand;
				*best_satellite = satellites[sat_ind];
				*time_of_arrival = time;
			}
		}

		time += time_step;
	}
}

	

double sattrack_get_khz_frequency(double time, const orbit_t *satellite){
	return (133*1000);
}

void sattrack_get_aziele(double time, const geodetic_t *qth_coord, const orbit_t *satellite, double *azimuth, double *elevation){
	double jul_utc = time + 2444238.5;
	vector_t obs_set;
	Calculate_Obs(jul_utc, satellite->position, satellite->velocity, qth_coord, &obs_set);
	*azimuth = obs_set.x;
	*elevation = obs_set.y;
}
	

double sattrack_get_max_elevation(double start_time, float time_step, const geodetic_t *qth_coord, const orbit_t *satellite){
	bool reached_los = false;
	float time = start_time;
	double max_ele = -1;
	while (!reached_los){
		double azimuth, elevation;
		orbit_predict(satellites[i], time);
		sattrack_get_aziele(time, qth_coord, satellites[i], &azimuth, &elevation);

		if (elevation > max_ele){
			max_ele = elevation;
		}

		if (elevation < ELE_THRESH){
			reached_los = true;
		}

		time += time_step;
	}
}
