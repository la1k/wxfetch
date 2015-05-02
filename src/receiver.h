#include <gnuradio/top_block.h>
#include <osmosdr/source.h>

typedef struct{
	osmosdr::source::sptr src;
	gr::top_block_sptr top_block;
} receiver_t;

void receiver_initialize(receiver_t *rec);
void receiver_set_filename(receiver_t *rec, const char* filename);
void receiver_set_frequency(receiver_t *rec, float freqv);

void receiver_start(receiver_t *rec);
void receiver_stop(receiver_t *rec);
