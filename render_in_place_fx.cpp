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

//registers the plugin with Wwise
AK_IMPLEMENT_PLUGIN_FACTORY(render_in_place_fx, AkPluginTypeEffect, render_in_place_config::company_id, render_in_place_config::plugin_id)

//constructor
render_in_place_fx::render_in_place_fx()
	: m_working_buffer(nullptr)
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
	render_in_place_fx_params* parameters = (render_in_place_fx_params*)params;

	m_recorder = static_cast<render_in_place_recorder*>(AK_PLUGIN_NEW(allocator, render_in_place_recorder()));

	//initializes the recorder with the files and sample rate
	m_recorder->initialize_stream(parameters->m_rtpc.file_name, format.uSampleRate);
	
	//initialize the working buffer
	//set to be the largest possible size for a sample (up to 512 samples per call to Execute)
	const AkUInt32 working_buffer_size = format.channelConfig.uNumChannels * sizeof(AkInt16) * context->GlobalContext()->GetMaxBufferLength();
	m_working_buffer = (AkInt16*)AK_PLUGIN_ALLOC(allocator, working_buffer_size);

	return AK_Success;
}

//called when plugin ends
AKRESULT render_in_place_fx::Term(AK::IAkPluginMemAlloc* allocator)
{
	AKRESULT result = AK_Success;

	//writes wav data to a new file
	if (!m_recorder->write_to_disk())
	{
		result = AK_Fail;
	}

	//frees the memory from the recorder
	if (m_recorder != nullptr)
	{
		AK_PLUGIN_FREE(allocator, m_recorder);
	}

	//frees the memory from the buffer
	if (m_working_buffer != nullptr)
	{
		AK_PLUGIN_FREE(allocator, m_working_buffer);
	}

	//deletes this plugin
	AK_PLUGIN_DELETE(allocator, this);
	return result;
}

AKRESULT render_in_place_fx::Reset()
{
	return AK_Success;
}

AKRESULT render_in_place_fx::GetPluginInfo(AkPluginInfo& out_plugin_info)
{
	//generic boilerplate settings for more fx plugins
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
		//gets the samples from the buffer for this given channel
		AkReal32* AK_RESTRICT samples = (AkReal32* AK_RESTRICT)buffer->GetChannel(i);

		//converts the samples from a float to a short since type short is the data needed to write into the PCM format for the wav file
		serialize_samples(samples, buffer->uValidFrames, channel_count, m_working_buffer + i);
	}

	//recorder accumulates the wav data, combining it with the wave data it already captured from previous samples
	m_recorder->record(m_working_buffer, buffer->uValidFrames * sizeof(short) * channel_count, channel_count);
}

AKRESULT render_in_place_fx::TimeSkip(AkUInt32 frames)
{
	//time skipping is not used for this plugin so a common result is returned
	return AK_DataReady;
}
