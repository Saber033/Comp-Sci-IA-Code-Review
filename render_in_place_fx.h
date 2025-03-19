#include "render_in_place_fx_params.h"

class render_in_place_recorder;

/// See https://www.audiokinetic.com/library/edge/?source=SDK&id=soundengine__plugins__effects.html
/// for the documentation about effect plug-ins
class render_in_place_fx
	: public AK::IAkInPlaceEffectPlugin
{
public:
	render_in_place_fx();
	~render_in_place_fx();

	/// Plug-in initialization.
	/// Prepares the plug-in for data processing, allocates memory and sets up the initial conditions.
	AKRESULT Init(AK::IAkPluginMemAlloc* allocator, AK::IAkEffectPluginContext* context, AK::IAkPluginParam* params, AkAudioFormat& format) override;

	/// Release the resources upon termination of the plug-in.
	AKRESULT Term(AK::IAkPluginMemAlloc* allocator) override;

	/// The reset action should perform any actions required to reinitialize the
	/// state of the plug-in to its original state (e.g. after Init() or on effect bypass).
	AKRESULT Reset() override;

	/// Plug-in information query mechanism used when the sound engine requires
	/// information about the plug-in to determine its behavior.
	AKRESULT GetPluginInfo(AkPluginInfo& out_plugin_info) override;

	/// Effect plug-in DSP execution.
	void Execute(AkAudioBuffer* buffer) override;

	/// Skips execution of some frames, when the voice is virtual playing from elapsed time.
	/// This can be used to simulate processing that would have taken place (e.g. update internal state).
	/// Return AK_DataReady or AK_NoMoreData, depending if there would be audio output or not at that point.
	AKRESULT TimeSkip(AkUInt32 frames) override;

private:
	render_in_place_fx_params* m_params;
	AkInt16* m_working_buffer; //this buffer contains the data converted from the Wwise format to the format used to write the file
	render_in_place_recorder* m_recorder; //the utility used for recording the data from Wwise, combining it and saving it to a new file
};
