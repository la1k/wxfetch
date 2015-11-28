#include <gnuradio/top_block.h>
#include <gnuradio/blocks/wavfile_sink.h>
#include <gnuradio/basic_block.h>
#include <gnuradio/qtgui/freq_sink_f.h>

typedef struct {
	gr::basic_block_sptr source_block;
	gr::top_block_sptr top_block;

	gr::qtgui::freq_sink_f::sptr freq_displayer;
} receiver_t;

enum receiver_type {
	RECV_RTL_SDR,
	RECV_AUDIOCARD
};

void receiver_initialize(receiver_t *receiver, enum receiver_type type);

void receiver_start(receiver_t *rec);
void receiver_stop(receiver_t *rec);
