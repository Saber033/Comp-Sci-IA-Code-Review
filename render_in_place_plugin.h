#include <AK/Wwise/Plugin.h>

//see https://www.audiokinetic.com/library/edge/?source=SDK&id=plugin__dll.html
//for the documentation about Authoring plug-ins
class render_in_place_plugin final
	: public AK::Wwise::Plugin::AudioPlugin
{
public:
	//constructor
	render_in_place_plugin();
	//destructor
	~render_in_place_plugin();

	//this function is called by Wwise to obtain parameters that will be written to a bank
	//because these can be changed at run-time, the parameter block should stay relatively small
	//larger data should be put in the Data Block
	bool GetBankParameters(const GUID& platform_guid, AK::Wwise::Plugin::DataWriter& data_writer) const override;
};

DECLARE_AUDIOPLUGIN_CONTAINER(render_in_place); //exposes our PluginContainer structure that contains the info for our plugin
