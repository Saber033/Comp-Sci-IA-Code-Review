// Takes a value in decibels and returns it in linear units
// $$from-petekug$$: I would mention that it is pre-fixed with "__declspec(dllexport)" so that it can be accessed from your unit tests
__declspec(dllexport) float db_to_linear_units(float db_value);

// $$from-petekug$$: missing description
void deserialize_samples(const short* buffer, int sample_count, int buffer_stride, float* out_samples);

// Takes an audio sample buffer stored in floating point format and convert to a 2 byte output buffer format
// $$from-petekug$$: I would mention that it is pre-fixed with "__declspec(dllexport)" so that it can be accessed from your unit tests
__declspec(dllexport) void serialize_samples(const float* samples, int sample_count, int buffer_stride, short* out_buffer);
