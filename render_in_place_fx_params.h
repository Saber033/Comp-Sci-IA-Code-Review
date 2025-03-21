#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Plugin/PluginServices/AkFXParameterChangeHandler.h>

//this is the data that can be changed from the tool, just the file name
struct render_in_place_rtpc_params
{
	char file_name[AK_MAX_PATH];
};

//id used to communicate which control was updated in the tool to the code
enum e_render_in_place_param_id
{
	authoring_file_name = 0,
	
	param_id_count
};

//updates the parameters in the plugin based on changes made to the soundbank file or through the authoring tool
struct render_in_place_fx_params
	: public AK::IAkPluginParam
{
	//constructor
	render_in_place_fx_params();

	//copy constructor
	render_in_place_fx_params(const render_in_place_fx_params& params);

	//destructor
	~render_in_place_fx_params();

	//create a duplicate of the parameter node instance in its current state
	IAkPluginParam* Clone(AK::IAkPluginMemAlloc* allocator) override;

	//initialize the plug-in parameter node interface.
	//initializes the internal parameter structure to default values or with the provided parameter block if it is valid
	AKRESULT Init(AK::IAkPluginMemAlloc* allocator, const void* params_block, AkUInt32 block_size) override;

	//called by the sound engine when a parameter node is terminated
	AKRESULT Term(AK::IAkPluginMemAlloc* allocator) override;

	//set all plug-in parameters at once using a parameter block
	AKRESULT SetParamsBlock(const void* params_block, AkUInt32 block_size) override;

	//update a single parameter at a time and perform the necessary actions on the parameter changes
	AKRESULT SetParam(AkPluginParamID param_id, const void* param_value, AkUInt32 param_size) override;

	static const AkUInt32 k_number_of_parameters= 1;

	//used to communicate changes captured from the tool to the render in place plugin classes
	AK::AkFXParameterChangeHandler<k_number_of_parameters> m_param_change_handler;

	//rtpc stands for "real time parameter controls" which are values that can be adjusted and updated in the code without reloading the tool
	render_in_place_rtpc_params m_rtpc;
};
