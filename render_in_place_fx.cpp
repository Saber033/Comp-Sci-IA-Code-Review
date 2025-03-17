#include "render_in_place_fx.h"
#include "render_in_place_config.h"
#include "render_in_place_utilities.h"
#include "render_in_place_recorder.h"

#include <AK/AkWwiseSDKVersion.h>

//creates an instance of the render in place fx plugin
AK::IAkPlugin* Createrender_in_place_fx(AK::IAkPluginMemAlloc* allocator)
{
	return AK_PLUGIN_NEW(allocator, render_in_place_fx());
}

//creates a new instance of render in place fx params
AK::IAkPluginParam* Createrender_in_place_fxParams(AK::IAkPluginMemAlloc* allocator)
{
	return AK_PLUGIN_NEW(allocator, render_in_place_fx_params());
}

// $$from-petekug$$: registers this plugin with Wwise
AK_IMPLEMENT_PLUGIN_FACTORY(render_in_place_fx, AkPluginTypeEffect, render_in_place_config::company_id, render_in_place_config::plugin_id)

//constructor
render_in_place_fx::render_in_place_fx()
	: m_params(nullptr)
	, m_allocator(nullptr)
	, m_context(nullptr)
	, m_working_buffer(nullptr)
	, m_recorder(nullptr)
{
}

//destructor
render_in_place_fx::~render_in_place_fx()
{
}

//called when plugin starts
AKRESULT render_in_place_fx::Init(AK::IAkPluginMemAlloc* allocator, AK::IAkEffectPluginContext* context, AK::IAkPluginParam* params, AkAudioFormat& format)
{
	//initialize member variables
	m_params = (render_in_place_fx_params*)params;
	m_allocator = allocator;
	m_context = context;

	m_recorder = static_cast<render_in_place_recorder*>(AK_PLUGIN_NEW(allocator, render_in_place_recorder()));

	//initializes the recorder with the files and sample rate
	m_recorder->initialize_stream(m_params->m_rtpc.file_name, format.uSampleRate);

	// $$from-petekug$$: we set it to the be the largest possible size for a sample (up to 512 samples per call to Execute)
	//initialize the working buffer
	const AkUInt32 working_buffer_size = format.channelConfig.uNumChannels * sizeof(AkInt16) * m_context->GlobalContext()->GetMaxBufferLength();
	m_working_buffer = (AkInt16*)AK_PLUGIN_ALLOC(m_allocator, working_buffer_size);

	return AK_Success;
}

//called when plugin ends
AKRESULT render_in_place_fx::Term(AK::IAkPluginMemAlloc* allocator)
{
	AKRESULT result = AK_Success;

	// $$from-petekug$$: writes the combined wav data recorded from wwise to a new wav file
	//writes the wav files to a new file
	if (!m_recorder->write_to_disk())
	{
		result = AK_Fail;
	}

	//frees the memory from the recorder
	if (m_recorder != nullptr)
	{
		AK_PLUGIN_FREE(m_allocator, m_recorder);
	}

	//frees the memory from the buffer
	if (m_working_buffer != nullptr)
	{
		AK_PLUGIN_FREE(m_allocator, m_working_buffer);
	}

	// $$from-petekug$$: deletes this plugin (the memory allocator still lives on for other calls in wwise)
	//deletes the memory allocator
	AK_PLUGIN_DELETE(allocator, this);
	return result;
}

AKRESULT render_in_place_fx::Reset()
{
	return AK_Success;
}

AKRESULT render_in_place_fx::GetPluginInfo(AkPluginInfo& out_plugin_info)
{
	// $$from-petekug$$: generic boilerplate settings for more fx plugins
	//saves the plugin info in variables
	out_plugin_info.eType = AkPluginTypeEffect;
	out_plugin_info.bIsInPlace = true;
	out_plugin_info.bCanProcessObjects = false;
	out_plugin_info.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
	return AK_Success;
}

void render_in_place_fx::Execute(AkAudioBuffer* buffer)
{
	//gets the channel count
	const AkUInt32 channel_count = buffer->NumChannels();

	//loops through the channels
	for (AkUInt32 i = 0; i < channel_count; i++)
	{
		// $$from-petekug$$: gets the samples from the buffer for this given channel (i.e. left speaker, right speaker, etc.)
		//gets the samples from the buffer
		AkReal32* AK_RESTRICT samples = (AkReal32* AK_RESTRICT)buffer->GetChannel(i);

		// $$from-petekug$$: why do we covert it to a short? (hint: short is the PCM format we use for our wave file)
		//converts the samples from a float to a short
		serialize_samples(samples, buffer->uValidFrames, channel_count, m_working_buffer + i);
	}

	// $$from-petekug$$: recorder accumuluates the wav data, combining it with the wave data it already captured from previous samples
	//recorder loads the wav file(s) and records the data
	m_recorder->record(m_working_buffer, buffer->uValidFrames * sizeof(short) * channel_count, channel_count);
}

AKRESULT render_in_place_fx::TimeSkip(AkUInt32 frames)
{
	// $$from-petekug$$: time skipping is not used for this plugin, so we just return a common result
	return AK_DataReady;
}
