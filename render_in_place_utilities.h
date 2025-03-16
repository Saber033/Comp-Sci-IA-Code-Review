// Takes a value in decibels and returns it in linear units
__declspec(dllexport) float db_to_linear_units(float db_value);

void deserialize_samples(const short* buffer, int sample_count, int buffer_stride, float* out_samples);

// Takes an audio sample buffer stored in floating point format and convert to a 2 byte output buffer format
__declspec(dllexport) void serialize_samples(const float* samples, int sample_count, int buffer_stride, short* out_buffer);
