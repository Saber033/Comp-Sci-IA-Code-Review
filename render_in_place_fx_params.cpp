#include "render_in_place_fx_params.h"
#include "render_in_place_config.h"
#include <AK/Tools/Common/AkBankReadHelpers.h>

render_in_place_fx_params::render_in_place_fx_params()
{
}

render_in_place_fx_params::~render_in_place_fx_params()
{
}

//copy constructor
render_in_place_fx_params::render_in_place_fx_params(const render_in_place_fx_params& params)
{
	m_rtpc = params.m_rtpc;
	m_param_change_handler.SetAllParamChanges();
}

//makes a copy of the fx params class
AK::IAkPluginParam* render_in_place_fx_params::Clone(AK::IAkPluginMemAlloc* allocator)
{
	return AK_PLUGIN_NEW(allocator, render_in_place_fx_params(*this));
}

//called when any audio asset with this plugin assigned to it is loaded in a game
AKRESULT render_in_place_fx_params::Init(AK::IAkPluginMemAlloc* allocator, const void* params_block, AkUInt32 block_size)
{
	AKRESULT result = AK_Success;

	// $$from-petekug$$: when the block size is 0, this is the first time this plugin has been called on this asset
	if (block_size == 0)
	{
		m_rtpc.file_name[0] = 0;
		m_param_change_handler.SetAllParamChanges();
		result = AK_Success;
	}
	else
	{
		// $$from-petekug$$: otherwise, we can load it with the data saved from the previous session
		result = SetParamsBlock(params_block, block_size);
	}

	return result;
}

//called when the class terminates
AKRESULT render_in_place_fx_params::Term(AK::IAkPluginMemAlloc* allocator)
{
	AK_PLUGIN_DELETE(allocator, this);
	return AK_Success;
}

//helper function that takes data from a part of a file and loads it into the memory of the class
AKRESULT render_in_place_fx_params::SetParamsBlock(const void* params_block, AkUInt32 block_size)
{
	AKRESULT result = AK_Success;

	// $$from-petekug$$: casting to an AkUint8*, in otherwords a byte buffer, for us to deserialize and extract the settings from
	AkUInt8* local_params_block = (AkUInt8*)params_block;

	// $$from-petekug$$: first we perform a quick compability test
	if (READBANKDATA(AkUInt16, local_params_block, block_size) != render_in_place_config::plugin_version)
	{
		result = AK_WrongBankVersion;
	}
	else
	{
		// $$from-petekug$$: I think you can just use local_params_block instead of this, but double check!
		unsigned char* p_params_block = (unsigned char*)params_block;

		//utility function that does a string copy from bank data to a char array
		COPYBANKSTRING_CHAR(p_params_block, block_size, m_rtpc.file_name, AK_MAX_PATH);

		//null terminate the strings
		m_rtpc.file_name[AK_MAX_PATH - 1] = '\0';

		//notifies all parts of the plugin that there's been an update to the parameters
		m_param_change_handler.SetAllParamChanges();
	}

	return result;
}

//called when a parameter is changed in the authoring tool api, never called in game unless there's a connection to the authoring tool established
AKRESULT render_in_place_fx_params::SetParam(AkPluginParamID in_paramID, const void* in_pValue, AkUInt32 in_ulParamSize)
{
	AKRESULT result = AK_Success;

	//finds the parameter that has been changed in the ui and sets it to the file
	if (in_paramID == e_render_in_place_param_id::authoring_file_name)
	{
		//data that has been sent by the tool and will be copied into the game data
		memcpy(m_rtpc.file_name, (const char*)in_pValue, AK_MAX_PATH);

		//null terminate string
		m_rtpc.file_name[AK_MAX_PATH - 1] = '\0';

		//notifies all parts of the plugin that there's been an update to the parameters
		m_param_change_handler.SetParamChange(in_paramID);
	}

	return result;
}
