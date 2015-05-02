#include <gnuradio/top_block.h>
#include <osmosdr/source.h>
#include <gnuradio/blocks/wavfile_sink.h>

typedef struct{
	osmosdr::source::sptr src;
	gr::top_block_sptr top_block;
	gr::blocks::wavfile_sink::sptr sink;
} receiver_t;

void receiver_initialize(receiver_t *rec);
void receiver_set_filename(receiver_t *rec, const char* filename);
void receiver_set_frequency(receiver_t *rec, float freqv);

void receiver_start(receiver_t *rec);
void receiver_stop(receiver_t *rec);
