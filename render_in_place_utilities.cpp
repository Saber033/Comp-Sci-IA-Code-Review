#include "render_in_place_utilities.h"
#include <math.h>
#include <iostream>

//takes in a value in db_units and performs a conversion to convert to linear units
float db_to_linear_units(float db_value)
{
	float result = 0.0f;

	//faster to multiply floats than divide
	result = powf(10.0f, db_value * 0.05f);

	return result;
}

//short to float conversion
void deserialize_samples(const short* buffer, int sample_count, int buffer_stride, float* out_samples)
{
	//loops through the sample count
	for (int i = 0; i < sample_count; i++)
	{
		short sample = *buffer;
		//cast to a float
		*out_samples = (float)(sample) / SHRT_MAX;

		//increase array index
		out_samples++;
		buffer += buffer_stride;
	}
}

//float to short conversion
void serialize_samples(const float* samples, int sample_count, int buffer_stride, short* out_buffer)
{
	//loop through the samples
	for (int i = 0; i < sample_count; i++)
	{
		float sample = *samples;

		//preventing overflow
		if (sample > 1.0f)
		{
			sample = 1.0f;
		}
		else if (sample < -1.0f)
		{
			sample = -1.0f;
		}

		//conversion to a short
		*out_buffer = (short)((float)(SHRT_MAX)*sample);

		//stepping through the array
		samples++;
		//have to step further through the output array due to it being a short and having a smaller size
		out_buffer += buffer_stride;
	}
}