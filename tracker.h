void sattrack_get_best_elevation(double time, double time_offset, double time_step, const geodetic_t *qth_coord, int num_satellites, const orbit_t **satellites, orbit_t *best_satellite, double *time_of_arrival);

double sattrack_get_khz_frequency(double time, const orbit_t *satellite);

void sattrack_get_aziele(double time, const geodetic_t *qth_coord, const orbit_t *satellite, double *azimuth, double *elevation); 


double sattrack_get_max_elevation(double start_time, const geodetic_t *qth_coord, orbit_t *satellite);
