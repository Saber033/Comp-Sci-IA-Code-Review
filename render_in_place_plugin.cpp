#include "render_in_place_plugin.h"
#include "render_in_place_config.h"
#include "render_in_place_fx_factory.h"

render_in_place_plugin::render_in_place_plugin()
{
}

render_in_place_plugin::~render_in_place_plugin()
{
}

//called when a change to the ui is being saved to the game data on the file system
bool render_in_place_plugin::GetBankParameters(const GUID& platform_guid, AK::Wwise::Plugin::DataWriter& data_writer) const
{
	//writes plugin version
	data_writer.WriteUInt16(render_in_place_config::plugin_version);
	//writes the string that is used for the filename when it is recorded
	data_writer.WriteString(m_propertySet.GetString(platform_guid, "AuthoringFilename"));

	return true;
}

DEFINE_AUDIOPLUGIN_CONTAINER(render_in_place); // Create a PluginContainer structure that contains the info for our plugin
EXPORT_AUDIOPLUGIN_CONTAINER(render_in_place); // This is a DLL, we want to have a standardized name
ADD_AUDIOPLUGIN_CLASS_TO_CONTAINER(            // Add our CLI class to the PluginContainer
	render_in_place,                           // Name of the plug-in container for this shared library
	render_in_place_plugin,                    // Authoring plug-in class to add to the plug-in container
	render_in_place_fx                         // Corresponding Sound Engine plug-in class
);
DEFINE_PLUGIN_REGISTER_HOOK

DEFINEDUMMYASSERTHOOK; // Placeholder assert hook for Wwise plug-ins using AKASSERT (cassert used by default)

