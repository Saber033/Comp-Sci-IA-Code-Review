#include "render_in_place_recorder.h"
#include <AK/SoundEngine/Common/AkSpeakerConfig.h>
#include <AK/SoundEngine/Common/IAkPlugin.h>

render_in_place_recorder::render_in_place_recorder() :
	m_audio_samples(),
	m_stream_file_path(nullptr),
	m_sample_rate(0) 
{ }

//loads the audio samples and saves in a buffer
bool render_in_place_recorder::record(void* buffer, unsigned int byte_count, unsigned int channel_count)
{
	return m_audio_samples.load_samples((short*)buffer, byte_count / sizeof(short) / channel_count, channel_count, m_sample_rate);
}

//writes the audio samples to a new file
bool render_in_place_recorder::write_to_disk()
{
	return m_audio_samples.write_samples(m_stream_file_path);
}

//initializes member variables
void render_in_place_recorder::initialize_stream(const char* stream_file_path, unsigned int sample_rate)
{
	m_stream_file_path = stream_file_path;
	m_sample_rate = sample_rate;
}