#include "audio_samples.h"
#include "render_in_place_utilities.h"
#include <AK/SoundEngine/Common/AkSpeakerConfig.h>

//This class creates a wav parser that is based on the wav file format documented on the page below
//https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html

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

//loads wav data from a file on disk
bool audio_samples::load_samples(const char *file_path)
{
	//reads a file in binary mode ("rb") in order to extract the binary data from the file
	//opening the file and creating a pointer to read through it
	FILE* read_ptr = fopen(file_path, "rb");
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

			//starts reading through to find the chunk header
			while (!fmt_chunk_found && fread(&fmt_chunk, sizeof(chunk_header), 1, read_ptr) != 0)
			{
				//looks for the format chunk header to see if it exists
				//keeps reading if true
				if (strncmp(fmt_chunk.chunk_id, "fmt", 3) != 0)
				{
					//examines a specific part of the file to locate the chunk id ("fmt")
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

				//limiting support to the basic pcm wav format, considering other extended formats in future updates
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
						//calculates the sample count
						unsigned int sample_count = data_chunk.chunk_size / ((m_bits_per_sample / 8) * m_channel_count);

						//sets a sample count to the one calculated in case multiple files are being loaded
						unsigned int current_sample_count = m_sample_count;

						//for combining multiple files
						m_sample_count += sample_count;

						//creates a buffer to hold the binary data from the wav file
						//size is divided by 2 as a short is half the size of a float
						short* buffer = new short[data_chunk.chunk_size / 2];

						//calculates a total chunk size that represents the size of everything in the file except for the header files
						//if it's 0 then it's set to the riff header data size, but if not that it adds itself to the data chunk size
						//if it's 0 then not combining multiple wav files yet
						m_total_chunk_size = m_total_chunk_size == 0 ? riff_header.data_size : m_total_chunk_size + data_chunk.chunk_size;

						//creates a new object that stores the samples in an array of floats
						//size is determined by how many samples need to be stored multiplied by the channel count to account for multiple audio channels
						float* new_samples = new float[m_sample_count * m_channel_count];
						float* new_samples_working_buffer = new_samples;

						//reads the data into the buffer
						fread(buffer, data_chunk.chunk_size, 1, read_ptr);

						//loops through all audio channels
						for (unsigned int i = 0; i < m_channel_count; i++)
						{
							//copies memory from m_samples to another samples buffer to hold past data when multiple files are loaded
							//copies the memory for each channel
							memcpy(new_samples_working_buffer, m_samples + current_sample_count * i, current_sample_count * sizeof(float));
							//working buffer is incremented for the next memcpy for the next channel
							new_samples_working_buffer += current_sample_count;

							//converts the binary data in the buffer to a float to be stored in the working buffer
							deserialize_samples(buffer + i, sample_count, m_channel_count, new_samples_working_buffer);
							new_samples_working_buffer += sample_count;
						}
						//deletes m_samples to stop memory leak and to because the old samples data is no longer needed
						delete[] m_samples;

						//sets the samples member variable to the new samples 
						m_samples = new_samples;

						//deleting the buffer to get rid of old data and to stop memory leak
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

//loads from multiple wav files
bool audio_samples::load_samples(const std::vector<const char *>& file_paths)
{
	bool success = true;

	//loops through all files
	for (int i = 0; i < file_paths.size(); i++)
	{
		//calls load samples on each file
		success &= audio_samples::load_samples(file_paths[i]);
	}

	return success;
}

//loading from a buffer already in memory loaded by the Wwise authoring tool, rather than from a file
bool audio_samples::load_samples(const short* stream, unsigned int sample_count, unsigned int channel_count, unsigned int sample_rate)
{
	bool success = false;

	//initializes the member variables
	m_sample_rate = sample_rate;

	//bits per sample is 16 to make sure the file is of PCM format (2 bytes)
	m_bits_per_sample = sizeof(short) * 8;
	m_channel_count = channel_count;

	//channel count multiplied by bytes per sample
	m_block_align = m_channel_count * m_bits_per_sample / 8;

	//block align multiplied by sample rate is the byte rate
	m_byte_rate = m_sample_rate * m_block_align;

	//1 is the PCM format code
	m_audio_format = 1;

	//creates a current sample count in case multiple files are being loaded
	unsigned int current_sample_count = m_sample_count;
	m_sample_count += sample_count;

	//represents the size of everything in the file 
	unsigned int current_total_chunk_size = (sizeof(wav_riff_header) + sizeof(chunk_header) + sizeof(wav_fmt_header) + sample_count * m_channel_count * sizeof(short)) - 8;

	//calculates a total chunk size that represents the size of everything in the file except for the header files
	//if it's 0 then it's set to the riff header data size, but if not that it adds itself to the data chunk size
	//if it's 0 it is the first time loading and has not yet combined multiple wav buffers
	m_total_chunk_size = m_total_chunk_size == 0 ? current_total_chunk_size : m_total_chunk_size + sample_count * m_channel_count * sizeof(short);

	//creates a samples array to hold the data extracted from the wav memory buffer from Wwise
	float* new_samples = new float[m_sample_count * m_channel_count];
	float* new_samples_working_buffer = new_samples;

	//loops through the channels
	for (unsigned int i = 0; i < m_channel_count; i++)
	{
		//copies memory from m_samples to another samples buffer to hold past data when multiple files are loaded
		//copies the memory for each channel
		memcpy(new_samples_working_buffer, m_samples + current_sample_count * i, current_sample_count * sizeof(float));
		new_samples_working_buffer += current_sample_count;

		//converts the data from the Wwise stream into a float value that can be stored into the buffer
		deserialize_samples(stream + i, sample_count, m_channel_count, new_samples_working_buffer);
		new_samples_working_buffer += sample_count;
	}

	//deletes the samples to avoid a memory leak and to get rid of the old data
	delete[] m_samples;

	//setting data to a temporary variable to store for later when multiple files are being loaded
	m_samples = new_samples;

	//everything ran correctly so it's successful
	success = true;

	//returns if it's successful or not
	return success;
}

bool audio_samples::write_samples(const char *file_path)
{
	//wb writes into files in binary mode as wav files store binary data
	//opens the file and sets a pointer to write inside it
	FILE* write_ptr = fopen(file_path, "wb");
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

		//initializes memory for a buffer array that will write to the file
		//divided by 2 as shorts are half the size of floats/ints
		short* buffer = new short[data_chunk.chunk_size / 2];

		//loops through the channels
		for (unsigned int i = 0; i < m_channel_count; i++)
		{
			//converts the samples from a float to binary data which can be written into a wav file
			serialize_samples(m_samples, m_sample_count, m_channel_count, buffer + i);
		}

		//writes the data to the wav file
		fwrite(buffer, data_chunk.chunk_size, 1, write_ptr);

		//buffer is deleted to avoid memory leaks and the temporary data is deleted as it's no longer needed
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
