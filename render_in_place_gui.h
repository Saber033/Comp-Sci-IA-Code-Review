#include "render_in_place_plugin.h"

//loads the controls to locate the render in place xml and creates controls for it in Wwise
class render_in_place_gui final
	: public AK::Wwise::Plugin::GUIWindows
{
public:
	//constructor
	render_in_place_gui();
};