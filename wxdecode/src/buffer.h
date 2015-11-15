#ifndef BUFFER_H_DEFINED
#define BUFFER_H_DEFINED

#include <pthread.h>

/**
 * Float buffer. Size is reduced when data is read, increased when data is written. Total size and internal data array is constant.
 **/
typedef struct {
	float *data; //data array
	int total_num_samples; //total allowable buffer size
	int current_start_position; //current start of buffered data
	int current_num_samples; //current length of buffered data
	pthread_mutex_t mutex;
} buffer_t;

/**
 * Initialize buffer instance. 
 *
 * \param buffer Buffer to initialize
 * \param tot_num_samples Total number of allowable samples in buffer
 **/
void buffer_initialize(buffer_t *buffer, int tot_num_samples);

/**
 * Free buffer instance. 
 *
 * \param buffer Buffer to free
 **/
void buffer_free(buffer_t *buffer);

/**
 * Write data to buffer in available space. 
 *
 * \param buffer Buffer
 * \param num_samples Number of samples in input data
 * \param samples Input data
 * \return Number of written samples
 **/
int buffer_fill(buffer_t *buffer, int num_samples, float *samples);

/**
 * Read data from buffer. The read data is removed from the buffer.
 *
 * \param buffer Buffer
 * \param num_samples Number of samples to read
 * \param samples Output data
 * \return Number of read samples
 **/
int buffer_read(buffer_t *buffer, int num_samples, float *samples);

#endif
