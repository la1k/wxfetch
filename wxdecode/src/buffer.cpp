//=======================================================================================================
// Copyright 2015 Asgeir Bjorgan
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT)
//=======================================================================================================

#include "buffer.h"
#include <stdlib.h>
#include <string.h>

void buffer_initialize(buffer_t *buffer, int tot_num_samples)
{
	buffer->data = (float*)malloc(sizeof(float)*tot_num_samples);
	buffer->total_num_samples = tot_num_samples;
	buffer->current_start_position = 0;
	buffer->current_num_samples = 0;

	pthread_mutex_init(&(buffer->mutex), NULL);
}

int buffer_fill(buffer_t *buffer, int num_samples, const float *samples)
{
	pthread_mutex_lock(&(buffer->mutex));

	//number of samples we can write to the buffer
	int capacity = buffer->total_num_samples - buffer->current_num_samples;
	int write_samples = num_samples;
	if (num_samples > capacity) {
		write_samples = capacity;
	}

	int first_part_len = write_samples;
	int second_part_len = 0;
	int end_space = buffer->total_num_samples - (buffer->current_start_position + buffer->current_num_samples);

	if (end_space <= 0) { //no space left at the end of the array
		first_part_len = 0;
		second_part_len = write_samples;
	} else if (end_space < first_part_len) { //some space at the end, some space at the beginning
		first_part_len = end_space;
		second_part_len = write_samples - end_space;
		end_space = 0;
	}

	if (first_part_len > 0) {
		memcpy(buffer->data + buffer->current_start_position + buffer->current_num_samples, samples, sizeof(float)*first_part_len);
	}

	if (second_part_len > 0) {
		int start_copy = abs(end_space);
		memcpy(buffer->data + start_copy, samples + first_part_len, sizeof(float)*second_part_len);
	}

	buffer->current_num_samples += write_samples;
	
	pthread_mutex_unlock(&(buffer->mutex));
	
	return write_samples;
}

void buffer_free(buffer_t *buffer)
{
	free(buffer->data);
	buffer->data = NULL;
	pthread_mutex_destroy(&(buffer->mutex));
}

int buffer_read(buffer_t *buffer, int num_samples, float *samples)
{
	pthread_mutex_lock(&(buffer->mutex));

	int read_samples = num_samples;
	if (num_samples > buffer->current_num_samples) {
		read_samples = buffer->current_num_samples;
	}

	int first_part_len = read_samples;
	int second_part_len = 0;
	if ((buffer->current_start_position + read_samples) > buffer->total_num_samples) {
		first_part_len = buffer->total_num_samples - buffer->current_start_position;
		second_part_len = read_samples - first_part_len;
	}
	
	if (first_part_len > 0) {
		memcpy(samples, buffer->data + buffer->current_start_position, sizeof(float)*first_part_len);
		buffer->current_num_samples -= first_part_len;
		buffer->current_start_position += first_part_len % buffer->total_num_samples;
	}

	if (second_part_len > 0) {
		memcpy(samples + first_part_len, buffer->data, sizeof(float)*second_part_len);
		buffer->current_num_samples -= second_part_len;
		buffer->current_start_position = second_part_len;
	}
	
	pthread_mutex_unlock(&(buffer->mutex));
	
	return read_samples;
}

int buffer_capacity(buffer_t *buffer)
{
	return buffer->total_num_samples - buffer->current_num_samples;
}

int buffer_length(buffer_t *buffer)
{
	return buffer->current_num_samples;
}
