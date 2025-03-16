#include "audio_samples.h"

struct AkChannelConfig;

namespace AK
{
	class IAkPluginMemAlloc;
}

class render_in_place_recorder
{
public:
	render_in_place_recorder();

	//loads samples for a specific set of audio samples
	bool record(void *buffer, unsigned int byte_count, unsigned int channel_count);

	//initializes member variables
	void initialize_stream(const char* stream_file_path, unsigned int sample_rate);

	//writes samples to a new file for a specific set of audio samples
	bool write_to_disk();

private:
	audio_samples m_audio_samples;
	const char* m_stream_file_path;
	unsigned int m_sample_rate;
};