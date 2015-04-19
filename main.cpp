
extern "C"{
#include <libpredict/orbit.h>
}
#include <iostream>
using namespace std;

int main(){
	const char *tle[2] = {
	"1 30000U 13010D   13 56.53554398  .00000000  00000-0 -11606-4 0    18",
	"2 30000  98.5602 246.6937 0004889 347.4954 227.2507 14.33925378    18"};

	struct orbit iss;
	orbit_init(&iss, tle);
	
	//QTH
	geodetic_t obs_geodetic;
	obs_geodetic.lat = 41.716905*deg2rad;
	obs_geodetic.lon = -72.727083*deg2rad;
	obs_geodetic.alt = 25.0/1000.0;
	obs_geodetic.theta = 0;

	for (;;) {
		//Get current time:
		double time = CurrentDaynum();
		double jul_utc = time + 2444238.5;

		orbit_predict(&iss, time);

		vector_t obs_set;
		Calculate_Obs(jul_utc, iss.position, iss.velocity, &obs_geodetic, &obs_set);
		cout << jul_utc << " " << iss.position[0] << " " << iss.position[1] << " " << iss.position[2] << " " << iss.velocity[0] << " " << iss.velocity[1] << " " << iss.velocity[2] << " " << obs_geodetic.lat << " " << obs_geodetic.lon << " " << obs_geodetic.alt << endl;

		cout << Degrees(obs_set.x) << " " << Degrees(obs_set.y) << endl;	

		//printf("lat: %f, lon: %f, alt: %f, ", Degrees(iss.latitude), Degrees(iss.longitude), iss.altitude);
		//printf("azi: %f, ele: %f, range: %f, range rate: %f\n ", Degrees(obs_set.x), Degrees(obs_set.y), obs_set.z, obs_set.w);

		//Sleep
		usleep(100000);
	}

	return 0;
}
