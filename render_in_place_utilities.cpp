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
// $$from-petekug$$: why do we have this function? (hint: used to take the format from the file system and capture into the format used by wwsie for digitial signal processing - i.e. for volume adjustment, reverberation, etc.)
void deserialize_samples(const short* buffer, int sample_count, int buffer_stride, float* out_samples)
{
	//loops through the sample count
	for (int i = 0; i < sample_count; i++)
	{
		short sample = *buffer;
		// $$from-petekug$$: (hint) we cast to a float and normalize to the range wwise expects it in, which is between -1 to 1
		//cast to a float
		*out_samples = (float)(sample) / SHRT_MAX;

		// $$from-petekug$$: kind of, but to be more accurate, we have written our current entry from our active channel to the output buffer, now we increment the location in the output buffer that we are writing to next
		//increase array index
		out_samples++;

		// $$from-petekug$$: and then jumping over a 'stride' in our input buffer, to make sure we are only capturing data for the sample channel (i.e. left, right, center speaker)
		buffer += buffer_stride;
	}
}

//float to short conversion
// $$from-petekug$$:  why do we have this function? (hint: used to take the format from that wwise does it's mixing in and converts it to the format that is used for a wave file on disk
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
		// $$from-petekug$$: (hint) we cast to a short and denormalize it from the range wwise expects, -1 to 1, to something symetrical with the min/max range for a short
		*out_buffer = (short)((float)(SHRT_MAX)*sample);

		//stepping through memory to the next location of the sample 
		samples++;

		// $$from-petekug$$: we jump by a 'stride', writing the current sample from wwise to the corresponding channel in the buffer we will write to the wave file 
		//have to step further through the output array due to it being a short and having a smaller size
		out_buffer += buffer_stride;
	}
}
