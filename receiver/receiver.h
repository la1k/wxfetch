#include <gnuradio/top_block.h>
#include <gnuradio/blocks/wavfile_sink.h>
#include <gnuradio/basic_block.h>
#include <gnuradio/qtgui/freq_sink_f.h>
#include <gnuradio/qtgui/waterfall_sink_f.h>
#include <apt/image_widget_f.h>
#include <apt/decode_ff.h>
#include <string>

typedef struct {
	gr::basic_block_sptr source_block;
	gr::top_block_sptr top_block;

	gr::qtgui::freq_sink_f::sptr freq_displayer;
	gr::qtgui::waterfall_sink_f::sptr waterfall;
	
	gr::apt::decode_ff::sptr wx_decoder;
	gr::apt::image_widget_f::sptr wx_widget;
} receiver_t;

enum receiver_type {
	RECV_RTL_SDR,
	RECV_AUDIOCARD,
	RECV_FIXED_FILE
};

void receiver_initialize(receiver_t *receiver, enum receiver_type type, std::string device_string);

void receiver_start(receiver_t *rec);
void receiver_stop(receiver_t *rec);
