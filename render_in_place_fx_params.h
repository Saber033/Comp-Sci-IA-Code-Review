#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Plugin/PluginServices/AkFXParameterChangeHandler.h>

struct render_in_place_rtpc_params
{
	char file_name[AK_MAX_PATH];
};

enum e_render_in_place_param_id
{
	authoring_file_name = 0,
	
	param_id_count
};

struct render_in_place_fx_params
	: public AK::IAkPluginParam
{
	render_in_place_fx_params();
	render_in_place_fx_params(const render_in_place_fx_params& params);

	~render_in_place_fx_params();

	/// Create a duplicate of the parameter node instance in its current state.
	IAkPluginParam* Clone(AK::IAkPluginMemAlloc* allocator) override;

	/// Initialize the plug-in parameter node interface.
	/// Initializes the internal parameter structure to default values or with the provided parameter block if it is valid.
	AKRESULT Init(AK::IAkPluginMemAlloc* allocator, const void* params_block, AkUInt32 block_size) override;

	/// Called by the sound engine when a parameter node is terminated.
	AKRESULT Term(AK::IAkPluginMemAlloc* allocator) override;

	/// Set all plug-in parameters at once using a parameter block.
	AKRESULT SetParamsBlock(const void* params_block, AkUInt32 block_size) override;

	/// Update a single parameter at a time and perform the necessary actions on the parameter changes.
	AKRESULT SetParam(AkPluginParamID param_id, const void* param_value, AkUInt32 param_size) override;

	static const AkUInt32 k_number_of_parameters= 1;
	AK::AkFXParameterChangeHandler<k_number_of_parameters> m_param_change_handler;

	render_in_place_rtpc_params m_rtpc;
};
