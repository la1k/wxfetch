#include <iostream>
#include <vector>
using namespace std;


#if 1
#include <gnuradio/top_block.h>
#include <osmosdr/source.h>
#include <gnuradio/blocks/wavfile_sink.h>
#include <gnuradio/blocks/wavfile_source.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/freq_xlating_fir_filter_ccf.h>
#include <gnuradio/filter/freq_xlating_fir_filter_fcf.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/filter/fir_filter_fff.h>
#include <gnuradio/filter/iir_filter_ffd.h>
#include <gnuradio/filter/rational_resampler_base_fff.h>
#include <gnuradio/blocks/multiply_const_ff.h>
#include <string>

int main(int argc, char*argv[]){
	//based on flow graph found on http://websterling.com/tsro/apt/
	gr::top_block_sptr tb = gr::make_top_block("test");

	//source
	string dev_str = "rtl=0,rtl_xtal=0,tuner_xtal=0,buffers=32,buflen=256kB,direct_samp=2,offset_tune=0";
	osmosdr::source::sptr src = osmosdr::source::make(dev_str);
	//gr::blocks::wavfile_source::sptr src = gr::blocks::wavfile_source::make(argv[1]);

	//sink
	gr::blocks::wavfile_sink::sptr sink = gr::blocks::wavfile_sink::make("/dev/null", 1, 11025, 16);
	
	////////////////////////////
	// PRE-PROCESSING FILTERS //
	////////////////////////////
	
	//frequency xlating FIR filter
	double sampling_freq = 96000;
	double cutoff_freq = 40000;
	double transition_width = 20000.0;
	vector<float> xlate_filter_taps = gr::filter::firdes::low_pass(1.0f, sampling_freq, cutoff_freq, transition_width, gr::filter::firdes::WIN_HAMMING);
	//gr::filter::freq_xlating_fir_filter_fcf::sptr xlate_filter = gr::filter::freq_xlating_fir_filter_fcf::make(1, xlate_filter_taps, 0, 96000);
	gr::filter::freq_xlating_fir_filter_ccf::sptr xlate_filter = gr::filter::freq_xlating_fir_filter_ccf::make(1, xlate_filter_taps, 0, 96000);

	//WBFM receive (from gr-analog/python/analog/wfm_rcv.py)
	double quad_rate = 96000;
	int audio_decimation = 5;
	double volume = 20.0;
	double max_dev = 75e3;
	double fm_demod_gain = quad_rate/(2.0*M_PI*max_dev);
	double audio_rate = quad_rate/(1.0*audio_decimation);
	gr::analog::quadrature_demod_cf::sptr fm_demod = gr::analog::quadrature_demod_cf::make(fm_demod_gain);
	vector<float> audio_coeffs = gr::filter::firdes::low_pass(1.0, quad_rate, audio_rate/2 - audio_rate/32, audio_rate/32, gr::filter::firdes::WIN_HAMMING);
	gr::filter::fir_filter_fff::sptr audio_filter = gr::filter::fir_filter_fff::make(audio_decimation, audio_coeffs);

	//fm_deemph (from gr-analog/python/analog/fm_emph.py)
	float tau = 75e-06;
	vector<double> btaps;
	float w_p = 1.0f/tau;
	float w_pp = tan(w_p/(audio_rate*2));
	btaps.push_back(w_pp/(1 + w_pp));
	btaps.push_back(btaps[0]);
	
	vector<double> ataps;
	ataps.push_back(1);
	ataps.push_back((w_pp - 1)/(w_pp + 1));
	gr::filter::iir_filter_ffd::sptr fm_deemph = gr::filter::iir_filter_ffd::make(btaps, ataps);

	//resamplers before audio output suitable for APTDEC
	gr::filter::rational_resampler_base_fff::sptr resamp_441khz = gr::filter::rational_resampler_base_fff::make(441, 192, vector<float>());
	gr::filter::rational_resampler_base_fff::sptr resamp_11025k = gr::filter::rational_resampler_base_fff::make(1, 4, vector<float>());
	gr::blocks::multiply_const_ff::sptr mult_const = gr::blocks::multiply_const_ff::make(6.5);
	

	//connect blocks
	tb->connect(src, 0, xlate_filter, 0);
	tb->connect(xlate_filter, 0, fm_demod, 0);
	tb->connect(fm_demod, 0, audio_filter, 0);
	tb->connect(audio_filter, 0, fm_deemph, 0);
	tb->connect(fm_deemph, 0, resamp_441khz, 0);
	tb->connect(resamp_441khz, 0, resamp_11025k, 0);
	tb->connect(resamp_11025k, 0, mult_const, 0);
	tb->connect(mult_const, 0, sink, 0);
	
	tb->run();

	//ok, jeg trenger egentlig ikke å bruke wait eller run i det hele tatt: kjør bare start, og gjør andre ting i mellomtiden... gnuradio kjører i bakgrunnen lell. 
	//Opprett en egen klasse som innkapsler alt. Ha medlemsfunksjoner for å sette relevante frekvenser og dritt, med lock underveis.
	//Trenger bare å vite hva jeg skal gjøre med utputten. 
		
}
#else

#include "tracker.h"


void wxsat_prepare(int *num_satellites, struct orbit*** satellites){
	const char *tle_noaa15[2] = {"1 25338U 98030A   15111.44744815  .00000353  00000-0  16813-3 0  9990",
	"2 25338  98.7719 110.0498 0010142 339.5996  20.4777 14.25603267880750"};
	const char *tle_noaa18[2] = {"1 28654U 05018A   15111.52351205  .00000264  00000-0  16910-3 0  9993",
	"2 28654  99.1819 102.7929 0015245 117.6325 242.6395 14.12181499511071"};
	const char *tle_noaa19[2] = {"1 33591U 09005A   15111.47492914  .00000377  00000-0  23086-3 0  9993",
	"2 33591  98.9850  61.2335 0013646 334.4839  25.5657 14.11916313319481"};

	*num_satellites = 3;
	*satellites = new struct orbit*[*num_satellites];
	(*satellites)[0] = new struct orbit;
	(*satellites)[1] = new struct orbit;
	(*satellites)[2] = new struct orbit;
	orbit_init((*satellites)[0], tle_noaa15);
	orbit_init((*satellites)[1], tle_noaa18);
	orbit_init((*satellites)[2], tle_noaa19);
}



int main(){
	//satellites
	int num_satellites = 0;
	struct orbit **satellites;
	wxsat_prepare(&num_satellites, &satellites);
	
	//QTH
	geodetic_t obs_geodetic;
	obs_geodetic.lat = 41.716905*deg2rad;
	obs_geodetic.lon = -72.727083*deg2rad;
	obs_geodetic.alt = 25.0/1000.0;
	obs_geodetic.theta = 0;


	double time_step = 1.0f/(24.0f*60);
	double curr_time = 0;
	const double DAYS_TO_SECONDS_FACTOR = 24*60*60;

	while (true){
		//estimate start and stop time of next satellite pass
		curr_time = CurrentDaynum();
		double time_of_arrival = 0;
		int sat_ind = 0;
		double time_of_departure = 0;
		sattrack_get_best_elevation(curr_time, 1, time_step, &obs_geodetic, num_satellites, satellites, &sat_ind, &time_of_arrival, &time_of_departure);

		double time_until_start_sec = (time_of_arrival - curr_time)*DAYS_TO_SECONDS_FACTOR;
		double duration_secs = (time_of_departure - time_of_arrival)*DAYS_TO_SECONDS_FACTOR;
		fprintf(stderr, "Calculated next satellite pass: cat %d in %f hours, lasting %f minutes.\n", satellites[sat_ind]->catnum, time_until_start_sec/(60*60), duration_secs/60);


		//sleep until approx. start
		sleep(time_until_start_sec);

		//satellite is here!
		
		sleep(duration_secs);

		//sleep some extra time just to shake off the last satellite. 
		sleep(60);
	}
	
	return 0;
}
#endif
