#include "audio_samples.h"

// $$from-petekug$$: you could mention something here about using forward declarations to keep the build times down
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
	audio_samples m_audio_samples;  // $$from-petekug$$: your utility class for capturing the samples from wwise and creating a file from the combined data 
	const char* m_stream_file_path; // $$from-petekug$$: the name of the file you are creating with the combined data being recorded
	unsigned int m_sample_rate; // $$from-petekug$$: the rate at which the captured samples will be written to the file
};
