
extern "C"{
#include <predict/orbit.h>
}

void sattrack_get_best_elevation(double time, double time_offset, double time_step, geodetic_t *qth_coord, int num_satellites, orbit **satellites, int *sat_ind, double *time_of_arrival, double *time_of_los);

double sattrack_get_max_elevation(double start_time, double time_step, geodetic_t *qth_coord, orbit *satellite, double *ret_sat_duration);
