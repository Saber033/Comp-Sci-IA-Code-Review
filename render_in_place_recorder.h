#include "audio_samples.h"

//utility class that accumulates audio samples and then writes them a wav file
class render_in_place_recorder
{
public:
	//constructor
	render_in_place_recorder();

	//loads samples for a specific set of audio samples
	bool record(void *buffer, unsigned int byte_count, unsigned int channel_count);

	//initializes member variables
	void initialize_stream(const char* stream_file_path, unsigned int sample_rate);

	//writes samples to a new file for a specific set of audio samples
	bool write_to_disk();

private:
	audio_samples m_audio_samples; //utility class for capturing the samples from Wwise and creating a file from the combined data
	const char* m_stream_file_path; //the name of the file that is created from the combined data that is recorded
	unsigned int m_sample_rate; //the rate at which the captured samples will be written to the file
};