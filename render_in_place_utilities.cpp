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
//used to take the format from the wav file and convert into the format used by Wwise for digital signal processing - i.e. for volume adjustment, reverberation, etc.
void deserialize_samples(const short* buffer, int sample_count, int buffer_stride, float* out_samples)
{
	//loops through the sample count
	for (int i = 0; i < sample_count; i++)
	{
		short sample = *buffer;
		//cast to a float and normalize to the range Wwise expects it in, which is between -1 to 1
		*out_samples = (float)(sample) / SHRT_MAX;

		//increment the location in the output buffer that will be written to next
		out_samples++;

		//jumping over a stride in the input buffer to make sure only capturing data for the sample channel
		buffer += buffer_stride;
	}
}

//float to short conversion 
//used to take the format used by Wwise to do the mixing and converts it oteh format that is used for a wav file
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

		//conversion to a short and denormalizes it from the range Wwise expects, -1 to 1, to something symmetrical with the min/max range for a short
		*out_buffer = (short)((float)(SHRT_MAX)*sample);

		//stepping through memory to the next location of the sample
		samples++;

		//jump by a stride, writing the current sample from Wwise to the corresponding channel in the buffer we will write to the wav file
		out_buffer += buffer_stride;
	}
}