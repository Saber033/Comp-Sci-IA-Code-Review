// Takes a value in decibels and returns it in linear units
//prefixed by __declspec(dllexport) so that it can be accessed from the unit tests
__declspec(dllexport) float db_to_linear_units(float db_value);

// Takes a Wwise buffer stored in short format and converts it to a 4 byte output buffer format of type float
void deserialize_samples(const short* buffer, int sample_count, int buffer_stride, float* out_samples);

// Takes an audio sample buffer stored in floating point format and convert to a 2 byte output buffer format
//prefixed by __declspec(dllexport) so that it can be accessed from the unit tests
__declspec(dllexport) void serialize_samples(const float* samples, int sample_count, int buffer_stride, short* out_buffer);
