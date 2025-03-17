#include "render_in_place_recorder.h"
#include <AK/SoundEngine/Common/AkSpeakerConfig.h>
#include <AK/SoundEngine/Common/IAkPlugin.h>

render_in_place_recorder::render_in_place_recorder() :
	m_audio_samples(),
	m_stream_file_path(nullptr),
	m_sample_rate(0) 
{ }

// $$from-petekug$$: I would mention why we save it in a buffer (hint: we save it into a buffer, accumulating it with previous samples, once the recording finishes, the buffer is ready to be written to a wav file combinging all the data)
//loads the audio samples and saves in a buffer
bool render_in_place_recorder::record(void* buffer, unsigned int byte_count, unsigned int channel_count)
{
	return m_audio_samples.load_samples((short*)buffer, byte_count / sizeof(short) / channel_count, channel_count, m_sample_rate);
}

// $$from-petekug$$: I would mention how this is writing all the combined buffers into that new file - this is the power of the new plugin you did this hard work for!
//writes the audio samples to a new file
bool render_in_place_recorder::write_to_disk()
{
	return m_audio_samples.write_samples(m_stream_file_path);
}

// $$from-petekug$$: Let's add a bit more detail to this one. Why are these two variables significant? You are capturing the name of the file you are going to create from the combined data, along with the rate at which you are recording the sample.
//initializes member variables
void render_in_place_recorder::initialize_stream(const char* stream_file_path, unsigned int sample_rate)
{
	m_stream_file_path = stream_file_path;
	m_sample_rate = sample_rate;
}
