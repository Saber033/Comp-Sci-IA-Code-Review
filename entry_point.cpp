#include "../liam_kugler_render_in_place/audio_samples.h"
#include "../liam_kugler_render_in_place/render_in_place_utilities.h"
#include <math.h>
#include <iostream>

static bool evaluate_test_result(bool succeeded, const char* test_case);
static bool test_db_to_linear_conversion();
static bool test_apply_gain();
static bool test_wave_loader();

// $$from-petekug$$: just a small comment for this would be helpful, something like, this is the entry point for where the unit tests are run from to validate that the core parts of the library work
int main(void)
{
	bool result= true;

	//will return a string depending on if the tests pass or not
	result &= test_db_to_linear_conversion();
	result &= test_apply_gain();
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

static bool evaluate_test_result(bool succeeded, const char* test_case)
{
	// $$from-petekug$$: this comment can be put on top of the function
	// $$from-petekug$$: it's not really a test, this is actually a helper function used to print the status of the test along with a name for it
	//takes in a function and a string to determine if the test passed or not
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

static bool test_db_to_linear_conversion()
{
	bool result= true;

	// $$from-petekug$$: this comment can be put on top of the function
	//tests the decibel to linear units conversion with a series of test cases
	result &= evaluate_test_result((db_to_linear_units(0.0f) == 1.0f), "Test for 0 dB (should be 1 in linear units)");
	result &= evaluate_test_result((db_to_linear_units(20.0f) == 10.0f), "Test for 20 dB (should be 10 in linear units)");
	result &= evaluate_test_result((db_to_linear_units(-20.0f) == 0.1f), "Test for -20 dB (should be 0.1 in linear units)");
	result &= evaluate_test_result((db_to_linear_units(-40.0f) == 0.01f), "Test for -40 dB (should be 0.01 in linear units)");

	return result;
}

// $$from-petekug$$: we should rename this test and give the function a comment
static bool test_apply_gain()
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

	// $$from-petekug$$: calls the serialize_samples function (we renamed it)
	//calls the apply gain function
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

static bool test_wave_loader()
{
	bool result= true;

	audio_samples samples;

	// $$from-petekug$$: I would put this comment at the top of the function instead of inside this code
	//calls the load samples and write samples on a mock wav file to see if the data is being correctly loaded and written

	// $$from-petekug$$: can these tests be brought back? 
	//samples.load_samples("C:\\Users\\lemur\\Downloads\\file_example_WAV_1MG.wav");
	//samples.load_samples("C:\\Users\\lemur\\Downloads\\file_example_WAV_1MG.wav");

	const std::vector<const char*> files = {"C:\\Users\\lemur\\Downloads\\file_example_WAV_1MG.wav", "C:\\Users\\lemur\\Downloads\\file_example_WAV_1MG.wav"};

	// $$from-petekug$$: should we check that load_samples and write_samples returns the expected result (true)?
	samples.load_samples(files);

	samples.write_samples("C:\\Users\\lemur\\Downloads\\new_written_wav_file.wav");

	return result;
}
