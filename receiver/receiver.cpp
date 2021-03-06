#include "receiver.h"
#include <vector>
#include <gnuradio/blocks/wavfile_sink.h>
#include <gnuradio/blocks/wavfile_source.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/freq_xlating_fir_filter_ccf.h>
#include <gnuradio/filter/freq_xlating_fir_filter_fcf.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/filter/fir_filter_fff.h>
#include <gnuradio/filter/iir_filter_ffd.h>
#include <gnuradio/filter/rational_resampler_base_fff.h>
#include <gnuradio/filter/fractional_resampler_ff.h>
#include <gnuradio/blocks/multiply_const_ff.h>
#include <gnuradio/blocks/wavfile_source.h>
#include <osmosdr/source.h>
#include <gnuradio/audio/source.h>
#include <gnuradio/audio/sink.h>
#include <string>
#include <iostream>
#include <QWidget>
using namespace std;

void receiver_initialize(receiver_t *rec, enum receiver_type type, std::string device_string){
	rec->top_block = gr::make_top_block("");

	//output audio
	gr::basic_block_sptr audio_block;
	int input_rate = 0;


	switch (type) {
		case RECV_RTL_SDR: {
			//based on flow graph found on http://websterling.com/tsro/apt/
			rec->source_block = osmosdr::source::make(device_string);

			////////////////////////////
			// PRE-PROCESSING FILTERS //
			////////////////////////////

			//frequency xlating FIR filter
			double sampling_freq = 96000;
			double cutoff_freq = 40000;
			double transition_width = 20000.0;
			vector<float> xlate_filter_taps = gr::filter::firdes::low_pass(1.0f, sampling_freq, cutoff_freq, transition_width, gr::filter::firdes::WIN_HAMMING);
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
			rec->top_block->connect(rec->source_block, 0, xlate_filter, 0);
			rec->top_block->connect(xlate_filter, 0, fm_demod, 0);
			rec->top_block->connect(fm_demod, 0, audio_filter, 0);
			rec->top_block->connect(audio_filter, 0, fm_deemph, 0);
			rec->top_block->connect(fm_deemph, 0, resamp_441khz, 0);
			rec->top_block->connect(resamp_441khz, 0, resamp_11025k, 0);
			rec->top_block->connect(resamp_11025k, 0, mult_const, 0);
			input_rate = 11025;

			audio_block = mult_const;
		break;
		}
		case RECV_AUDIOCARD: {
			input_rate = 44100;
			rec->source_block = gr::audio::source::make(input_rate, device_string);
			audio_block = rec->source_block;
		break;
		}
		case RECV_FIXED_FILE: {
			gr::blocks::wavfile_source::sptr wavfile_source = gr::blocks::wavfile_source::make(device_string.c_str(), true);
			rec->source_block = wavfile_source;
			audio_block = rec->source_block;
			input_rate = wavfile_source->sample_rate();
		break;
		}
	}

	//frequency displayer
	int fft_size = 2000;
	rec->freq_displayer = gr::qtgui::freq_sink_f::make(fft_size, gr::filter::firdes::WIN_HAMMING, 22500, 44100, "greier");
	rec->freq_displayer->set_plot_pos_half(true);

	//waterfall display
	rec->waterfall = gr::qtgui::waterfall_sink_f::make(fft_size, gr::filter::firdes::WIN_HAMMING, 22500, 44100, "waterfall");
	rec->waterfall->set_plot_pos_half(true);

	//APT decoding
	rec->wx_decoder = gr::apt::decode_ff::make();
	rec->wx_widget = gr::apt::image_widget_f::make();

	//audiocard sink
	unsigned int output_rate = 44100;
	gr::filter::fractional_resampler_ff::sptr resampler = gr::filter::fractional_resampler_ff::make(0, input_rate/(1.0f*output_rate)); 
	gr::audio::sink::sptr audio_sink = gr::audio::sink::make(output_rate, "default:CARD=PCH", true);

	//connect blocks
	rec->top_block->connect(audio_block, 0, rec->freq_displayer, 0);
	rec->top_block->connect(audio_block, 0, rec->waterfall, 0);
	rec->top_block->connect(audio_block, 0, rec->wx_decoder, 0);
	rec->top_block->connect(audio_block, 0, resampler, 0);
	rec->top_block->connect(resampler, 0, audio_sink, 0);
	rec->top_block->connect(rec->wx_decoder, 0, rec->wx_widget, 0);
}

void receiver_set_frequency(receiver_t *rec, float freqv){
	//rec->src->set_center_freq(freqv);
}

void receiver_start(receiver_t *rec){
	rec->top_block->start();
}

void receiver_stop(receiver_t *rec){
	rec->top_block->stop();

	//wait for top block to stop
	rec->top_block->wait();
}
