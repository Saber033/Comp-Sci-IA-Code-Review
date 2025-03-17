#include "audio_samples.h"
#include "render_in_place_utilities.h"
#include <AK/SoundEngine/Common/AkSpeakerConfig.h>

// $$from-petekug$$: you should mention why this url is here. something like how this class implements a parser based on the wave file format documented on that page.
//https://www.mmsp.ece.mcgill.ca/Documents/AfudioFormats/WAVE/WAVE.html

//constructor
audio_samples::audio_samples() : 
	m_samples(nullptr),
	m_sample_count(0),
	m_sample_rate(0),
	m_bits_per_sample(0),
	m_channel_count(0),
	m_audio_format(0),
	m_byte_rate(0),
	m_block_align(0),
	m_total_chunk_size(0) { }

//destructor
audio_samples::~audio_samples()
{
	delete[] m_samples;
	m_samples = nullptr;
}
// $$from-petekug$$: don't forget a comment for this function, maybe just something saying how it is responsible for loading wav data from a file on disk.
bool audio_samples::load_samples(const char *stream_file_path)
{
	// $$from-petekug$$: mention why you are using "rb"? hint, "rb" represents "opening a file for reading in binary mode"
	//opening the file and creating a pointer to read through it
	FILE* read_ptr = fopen(stream_file_path, "rb");
	bool success = false;

	//read through the file
	if (read_ptr != nullptr)
	{
		//reads through the first part of the wav file known as a riff header
		wav_riff_header riff_header;
		fread(&riff_header, sizeof(wav_riff_header), 1, read_ptr);
		
		//checks if the riff header id and format is found to determine if the file is in fact a wav file
		//will keep reading through if true
		if (strncmp(riff_header.id, "RIFF", 4) == 0 && strncmp(riff_header.format, "WAVE", 4) == 0)
		{
			chunk_header fmt_chunk;
			bool fmt_chunk_found = false;

			// $$from-petekug$$: which chunk header are we looking for? the format chunk? And what is special about the format chunk header (what does it have in it that we care about for our wave data)?
			//starts reading through to find the chunk header
			while (!fmt_chunk_found && fread(&fmt_chunk, sizeof(chunk_header), 1, read_ptr) != 0)
			{
				//looks for the chunk header to see if it exists
				//keeps reading if true
				if (strncmp(fmt_chunk.chunk_id, "fmt", 3) != 0)
				{
					//examines a specific part of the file to locate the chunk header
					fseek(read_ptr, fmt_chunk.chunk_size, SEEK_CUR);
				}
				else
				{
					fmt_chunk_found = true;
				}
			}

			if (fmt_chunk_found)
			{
				//begins reading to find the wav fmt header
				wav_fmt_header fmt_header;
				fread(&fmt_header, sizeof(fmt_header), 1, read_ptr);
				
				//initializes member variables from the fmt header
				m_sample_rate = fmt_header.sample_rate;
				m_bits_per_sample = fmt_header.bits_per_sample;
				m_channel_count = fmt_header.num_channels;
				m_block_align = fmt_header.block_align;
				m_byte_rate = fmt_header.byte_rate;
				m_audio_format = fmt_header.audio_format;

				// $$from-petekug$$: consider mentioning why only support this type, something like, limiting our support to the basic pcm wav format for practicality, consider other extended format in future updates
				//checks to ensure only the pcm wav format is supported, look at this for more detail: https://fileinfo.com/extension/pcm
				if (m_bits_per_sample == 16)
				{
					bool data_chunk_found = false;
					chunk_header data_chunk;

					//reads to find the data chunk, the part that plays the sounds and holds the sound data
					while (!data_chunk_found && fread(&data_chunk, sizeof(chunk_header), 1, read_ptr) != 0)
					{
						if (strncmp(data_chunk.chunk_id, "data", 4) != 0)
						{
							//searches a specific part of the file to determine if the data chunk exists
							fseek(read_ptr, data_chunk.chunk_size, SEEK_CUR);
						}
						else
						{
							data_chunk_found = true;
						}
					}

					if (data_chunk_found)
					{
						// $$from-petekug$$: consider spacing these out a little bit with newlines, making it easier to read the comments and to know what code they are describing
						
						//calculates the sample count
						unsigned int sample_count = data_chunk.chunk_size / ((m_bits_per_sample / 8) * m_channel_count);
						//sets a sample count to the one calculated in case multiple files are being loaded
						unsigned int current_sample_count = m_sample_count;
						//for combining multiple files
						m_sample_count += sample_count;

						// $$from-petekug$$: I would mention why the chunk size is divided by 2 for this buffer (hint: chunk size is in bytes, and there are 2 bytes in a short, and since the buffer is a short instead of a byte buffer, it only needs to be half the length)
						//creates a buffer to hold the binary data from the wav file
						short* buffer = new short[data_chunk.chunk_size / 2];

						// $$from-petekug$$: why do we do this when it's 0 (hint: because when it's 0, we know that it's the first time and aren't yet combinging mutiple wav files)? 
						//calculates a total chunk size that represents the size of everything in the file except for the header files
						//if it's 0 then it's set to the riff header data size, but if not that it adds itself to the data chunk size
						m_total_chunk_size = m_total_chunk_size == 0 ? riff_header.data_size : m_total_chunk_size + data_chunk.chunk_size;

						// $$from-petekug$$: I would make your pointer syntax consistent, either stick with "float *" or "float* "
						//creates a new object that stores the samples in an array of floats
						//size is determined by how many samples need to be stored multiplied by the channel count to account for multiple audio channels
						float *new_samples = new float[m_sample_count * m_channel_count];
						float* new_samples_working_buffer = new_samples;

						// $$from-petekug$$: what are we reading the data into?
						//reads the data
						fread(buffer, data_chunk.chunk_size, 1, read_ptr);

						//loops through all audio channels
						for (unsigned int i = 0; i < m_channel_count; i++)
						{
							// $$from-petekug$$: let's chat about this one after school to make the description a bit clearer
							//copies memory from m_samples to the new samples working buffer
							//has to account for multiple audio channels so sample count is multiplied by i
							//samples is added to sample count in case multiple files are loaded and existing data is already being stored
							memcpy(new_samples_working_buffer, m_samples + current_sample_count * i, current_sample_count * sizeof(float));

							//working buffer is incremented for the next memcpy for the next channel
							new_samples_working_buffer += current_sample_count;

							//converts the binary data in the buffer to a float to be stored in the working buffer
							deserialize_samples(buffer + i, sample_count, m_channel_count, new_samples_working_buffer);

							// $$from-petekug$$: let's add a comment for this one
							new_samples_working_buffer += sample_count;
						}

						// $$from-petekug$$: kind of, it does prevent a memory leak, but we are also deleting it b/c we don't need the old data anymore and will be overwriting it to point at the new memory buffer we have allocated
						//deletes m_samples to stop memory leak
						delete[] m_samples;

						//sets the samples member variable to the new samples 
						m_samples = new_samples;

						// $$from-petekug$$: what is being deleted here?
						delete[] buffer;

						//if everything is correctly read then the function is successful
						success = true;
					}
				}
			}
		}

		//closes the file
		fclose(read_ptr);
	}

	//returns if the loading was successful or not
	return success;
}

// $$from-petekug$$: don't forget a comment for this function, maybe something about loading from multiple files?
// $$from-petekug$$: do we want to change this variable name to be something that is plural, since it represents multiple file paths?
bool audio_samples::load_samples(const std::vector<const char *>& stream_file_path)
{
	bool success = true;

	//loops through all files
	for (int i = 0; i < stream_file_path.size(); i++)
	{
		//calls load samples on each file
		// $$from-petekug$$: I think it would be worth mentioning why you are calling &= here
		success &= audio_samples::load_samples(stream_file_path[i]);
	}

	return success;
}

// $$from-petekug$$: don't forget a comment for this function, maybe something about loading from a buffer already in memory loaded by the Wwise authoring tool, rather than loaded from a file
bool audio_samples::load_samples(const short* stream, unsigned int sample_count, unsigned int channel_count, unsigned int sample_rate)
{
	bool success = false;

	//initializes the member variables
	m_sample_rate = sample_rate;

	// $$from-petekug$$: also might be worth mentioning that there are 8 bits in a byte, and each sample for the standard PCM format is 2 bytes
	//bits per sample is 16 to make sure the file is of PCM format
	m_bits_per_sample = sizeof(short) * 8;

	m_channel_count = channel_count;

	// $$from-petekug$$: did you mean multiplied by bytes per sample (bits per sample / 8 == bytes per sample)?
	//channel count divided by bytes per sample
	m_block_align = m_channel_count * m_bits_per_sample / 8;

	//block align multiplied by sample rate is the byte rate
	m_byte_rate = m_sample_rate * m_block_align;

	// $$from-petekug$$: we should add a comment here, 1 just represents the PCM format code
	m_audio_format = 1;

	//creates a current sample count in case multiple files are being loaded
	unsigned int current_sample_count = m_sample_count;
	m_sample_count += sample_count;

	//represents the size of everything in the file 
	unsigned int current_total_chunk_size = (sizeof(wav_riff_header) + sizeof(chunk_header) + sizeof(wav_fmt_header) + sample_count * m_channel_count * sizeof(short)) - 8;

	// $$from-petekug$$: why do we do this when it's 0 (hint: because when it's 0, we know that it's the first time and aren't yet combinging mutiple wav buffers)? 
	//calculates a total chunk size that represents the size of everything in the file except for the header files
	//if it's 0 then it's set to the riff header data size, but if not that it adds itself to the data chunk size
	m_total_chunk_size = m_total_chunk_size == 0 ? current_total_chunk_size : m_total_chunk_size + sample_count * m_channel_count * sizeof(short);

	// $$from-petekug$$: in this case, we are getting it from the wav memory buffer from wwise (as opposed to the wav file)
	//creates a samples array to hold the data extracted from the wav file
	float* new_samples = new float[m_sample_count * m_channel_count];
	float* new_samples_working_buffer = new_samples;

	//loops through the channels
	for (unsigned int i = 0; i < m_channel_count; i++)
	{
		// $$from-petekug$$: let's chat about this one after school to make the description a bit clearer
		
		//copies memory from m_samples to the new samples working buffer
		//has to account for multiple audio channels so sample count is multiplied by i
		//samples is added to sample count in case multiple files are loaded and existing data is already being stored
		memcpy(new_samples_working_buffer, m_samples + current_sample_count * i, current_sample_count * sizeof(float));
		new_samples_working_buffer += current_sample_count;

		//converts the data from the Wwise stream into a float value that can be stored into the buffer
		deserialize_samples(stream + i, sample_count, m_channel_count, new_samples_working_buffer);
		new_samples_working_buffer += sample_count;
	}

	// $$from-petekug$$: kind of, it does prevent a memory leak, but we are also deleting it b/c we don't need the old data anymore and will be overwriting it to point at the new memory buffer we have allocated
	//deletes the samples to avoid a memory leak
	delete[] m_samples;

	// $$from-petekug$$: don't forget a comment
	m_samples = new_samples;

	// $$from-petekug$$: don't forget a comment
	success = true;

	// $$from-petekug$$: don't forget a comment
	return success;
}

bool audio_samples::write_samples(const char *stream_file_path)
{
	// $$from-petekug$$: mention why you are using "wb"? hint, "rb" represents "opening a file for writing in binary mode"
	//opens the file and sets a pointer to write inside it
	FILE* write_ptr = fopen(stream_file_path, "wb");
	bool success = false;

	//writes in the file
	if (write_ptr != nullptr)
	{
		wav_riff_header riff_header;
		//sets the riff header data size to the chunk size of everything in the file
		riff_header.data_size = m_total_chunk_size;
		//wav file id
		strncpy(riff_header.id, "RIFF", 4);
		//wav file format
		strncpy(riff_header.format, "WAVE", 4);
		//writes the riff header to the new file
		fwrite(&riff_header, sizeof(wav_riff_header), 1, write_ptr);

		chunk_header chunks;
		//chunk header id
		strncpy(chunks.chunk_id, "fmt ", 4);
		//chunk header size
		chunks.chunk_size = sizeof(wav_fmt_header);
		//writes the data from the chunk header into the new file
		fwrite(&chunks, sizeof(chunk_header), 1, write_ptr);

		wav_fmt_header fmt_header;
		//initializes all member variables under the format header
		fmt_header.audio_format = m_audio_format;
		fmt_header.bits_per_sample = m_bits_per_sample;
		fmt_header.block_align = m_block_align;
		fmt_header.byte_rate = m_byte_rate;
		fmt_header.num_channels = m_channel_count;
		fmt_header.sample_rate = m_sample_rate;
		//writes the wav fmt header to the new file
		fwrite(&fmt_header, sizeof(wav_fmt_header), 1, write_ptr);

		chunk_header data_chunk;
		//chunk id
		strncpy(data_chunk.chunk_id, "data", 4);
		//calculates the data chunk size using algebraic manipulation
		data_chunk.chunk_size = m_sample_count * ((m_bits_per_sample / 8) * m_channel_count);
		//writes the chunk header to the new file
		fwrite(&data_chunk, sizeof(chunk_header), 1, write_ptr);

		// $$from-petekug$$: I would mention why the chunk size is divided by 2 for this buffer (hint: chunk size is in bytes, and there are 2 bytes in a short, and since the buffer is a short instead of a byte buffer, it only needs to be half the length)
		//initializes memory for a buffer array that will write to the file
		short* buffer = new short[data_chunk.chunk_size / 2];

		//loops through the channels
		for (unsigned int i = 0; i < m_channel_count; i++)
		{
			//converts the samples from a float to binary data which can be written into a wav file
			serialize_samples(m_samples, m_sample_count, m_channel_count, buffer + i);
		}

		//writes the data to the wav file
		fwrite(buffer, data_chunk.chunk_size, 1, write_ptr);

		// $$from-petekug$$: yeah, we avoid memory leaks, and it's just a temporary working buffer that we don't need to keep around anymore after we poop all the data to the file.
		//buffer is deleted to avoid memory leaks
		delete[] buffer;

		//file is closed
		fclose(write_ptr);

		//successfully written the file
		success = true;
	}
	
	return success;
}

//returns a sample at a specific sample index
float audio_samples::get_sample(unsigned sample_index) const
{
	float sample = 0;	

	//checking if sample index is in the bounds of the array
	if (sample_index >= 0 && sample_index < m_sample_count)
	{
		sample = m_samples[sample_index];
	}

	return sample;
}

//returns sample count
unsigned int audio_samples::get_sample_count() const
{
	return m_sample_count;
}

//returns sample rate
unsigned int audio_samples::get_sample_rate() const
{
	return m_sample_rate;
}

//returns bits per sample
unsigned int audio_samples::get_bits_per_sample() const
{
	return m_bits_per_sample;
}

//returns channel count
unsigned int audio_samples::get_channel_count() const
{
	return m_channel_count;
}

//returns the sample count divided by the sample rate divided by the channel count 
//equals samples/second
unsigned int audio_samples::get_duration_seconds() const
{
	unsigned int duration = 0;

	//making sure there is no division by 0
	if (m_channel_count != 0 && m_sample_rate != 0)
	{
		duration = m_sample_count / m_sample_rate / m_channel_count;
	}

	return duration;
}
