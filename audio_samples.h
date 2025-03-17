#include <vector>

struct AkChannelConfig;

// $$from-petekug$$: I would mention that it is pre-fixed with "__declspec(dllexport)" so that it can be accessed from your unit tests
// $$from-petekug$$: it's also worth adding a description of this class. something like how it is a class that contains information from loaded wav data, either from wwise or a file. Allows for multiple wav files or data to be loaded and it will combine them into a single wav file when write_samples is called
class __declspec(dllexport) audio_samples
{
public:
	audio_samples();

	~audio_samples();

	// loads the audio file, wav format, into a deserialized format
	bool load_samples(const char* stream_file_path);

	// loads the audio files, wav format, into a combined deserialized format
	bool load_samples(const std::vector<const char*>& stream_file_path);

	// loads the audio stream, wwise stream format, into a deserialized format
	bool load_samples(const short* stream, unsigned int sample_count, unsigned int channel_count, unsigned int sample_rate);

	// writes the deserialized format into an output wave file
	bool write_samples(const char* stream_file_path);

	// returns a sample, given the index into the sample buffer
	float get_sample(unsigned int sample_index) const;

	// returns the number of samples in the audio file
	unsigned int get_sample_count() const;

	// returns the sample rate
	unsigned int get_sample_rate() const;

	// returns the bit count per sample
	unsigned int get_bits_per_sample() const;

	// returns the number of channels
	unsigned int get_channel_count() const;

	// returns the duration of the samples in seconds
	unsigned int get_duration_seconds() const;

private:
// $$from-petekug$$: comment needed, something like, by removing the padding the compiler adds to the data, we can read and write to the binary wav format directly with less code
#pragma pack(push, 1) 
	struct wav_riff_header
	{
		char id[4];
		unsigned int data_size;
		char format[4];
	};

	struct chunk_header
	{
		char chunk_id[4];
		unsigned int chunk_size;
	};

	struct wav_fmt_header
	{
		unsigned short audio_format;
		unsigned short num_channels;
		unsigned int sample_rate;
		unsigned int byte_rate;
		unsigned short block_align;
		unsigned short bits_per_sample;
	};
#pragma pack(pop)

	float* m_samples;

	unsigned int m_sample_count;
	unsigned int m_sample_rate;
	unsigned int m_bits_per_sample;
	unsigned int m_channel_count;
	unsigned short m_audio_format;
	unsigned int m_byte_rate;
	unsigned short m_block_align;
	unsigned int m_total_chunk_size;
};
