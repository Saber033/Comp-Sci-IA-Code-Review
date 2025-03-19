#include "../liam_kugler_render_in_place/audio_samples.h"
#include "../liam_kugler_render_in_place/render_in_place_utilities.h"
#include <math.h>
#include <iostream>

static bool evaluate_test_result(bool succeeded, const char* test_case);
static bool test_db_to_linear_conversion();
static bool test_serialize_samples();
static bool test_wave_loader();

//this is the entry point where the unit tests are run to validate that the core parts of the library work
int main(void)
{
	bool result= true;

	//will return a string depending on if the tests pass or not
	result &= test_db_to_linear_conversion();
	result &= test_serialize_samples();
	result &= test_wave_loader();

	if (result)
	{
		std::cout << "ALL TESTS PASSED!" << std::endl;
	}
	else
	{
		std::cout << "THERE WAS A TEST FAILURE, PLEASE FIX!" << std::endl;
	}

	return 0;
}

//helper function that is used to print the status of the test along with it's name
static bool evaluate_test_result(bool succeeded, const char* test_case)
{
	if (succeeded)
	{
		std::cout << "TEST: '" << test_case << "' PASSED." << std::endl;
	}
	else
	{
		std::cout << "TEST: '" << test_case << "' FAILED." << std::endl;
	}

	return succeeded;
}

//tests the decibel to linear units conversion with a series of test cases
static bool test_db_to_linear_conversion()
{
	bool result= true;

	result &= evaluate_test_result((db_to_linear_units(0.0f) == 1.0f), "Test for 0 dB (should be 1 in linear units)");
	result &= evaluate_test_result((db_to_linear_units(20.0f) == 10.0f), "Test for 20 dB (should be 10 in linear units)");
	result &= evaluate_test_result((db_to_linear_units(-20.0f) == 0.1f), "Test for -20 dB (should be 0.1 in linear units)");
	result &= evaluate_test_result((db_to_linear_units(-40.0f) == 0.01f), "Test for -40 dB (should be 0.01 in linear units)");

	return result;
}

//tests to see if an example array of samples of type float can be converted into shorts with no data loss
static bool test_serialize_samples()
{
	bool result= true;

	//creates an array of samples
	const int sample_count = 4;
	float samples[sample_count] =
	{
		 1.0f,
		 0.5f,
		 0.2f,
		-1.0f,
	};

	//creates an output buffer
	short output_buffer[sample_count] = {};

	//calls the serialize samples function
	serialize_samples(&samples[0], sample_count, 1, &output_buffer[0]);

	//manually calculates the values for an expected output buffer
	short expected_output_buffer[sample_count] =
	{
		static_cast<short>(1.0f * SHRT_MAX),
		static_cast<short>(0.5f * SHRT_MAX),
		static_cast<short>(0.2f * SHRT_MAX),
		static_cast<short>(-1.0f * SHRT_MAX),
	};

	//loops through the samples to see if the function works
	for (int i = 0; i < sample_count; i++)
	{
		//if the buffers are the same returns true
		result &= evaluate_test_result(output_buffer[i] == expected_output_buffer[i], "Comparing if the buffers are the same");
	}

	return result;
}

//calls the load samples and write samples on a mock wav file to see if the data is being correctly loaded and written 
static bool test_wave_loader()
{
	bool result= true;

	audio_samples samples;

	samples.load_samples("C:\\Users\\lemur\\Downloads\\file_example_WAV_1MG.wav");
	samples.load_samples("C:\\Users\\lemur\\Downloads\\file_example_WAV_1MG.wav");

	const std::vector<const char*> files = {"C:\\Users\\lemur\\Downloads\\file_example_WAV_1MG.wav", "C:\\Users\\lemur\\Downloads\\file_example_WAV_1MG.wav"};
	samples.load_samples(files);

	samples.write_samples("C:\\Users\\lemur\\Downloads\\new_written_wav_file.wav");

	return result;
}