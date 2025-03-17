#include "render_in_place_gui.h"

render_in_place_gui::render_in_place_gui()
{
}

ADD_AUDIOPLUGIN_CLASS_TO_CONTAINER(
	render_in_place,        // Name of the plug-in container for this shared library
	render_in_place_gui,    // Authoring plug-in class to add to the plug-in container
	render_in_place_fx      // Corresponding Sound Engine plug-in class
);
