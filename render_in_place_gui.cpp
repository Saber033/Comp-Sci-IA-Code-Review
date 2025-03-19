#include "render_in_place_gui.h"

//constructor
render_in_place_gui::render_in_place_gui()
{
}

ADD_AUDIOPLUGIN_CLASS_TO_CONTAINER(
	render_in_place,			//name of the plug-in container for this shared library
	render_in_place_gui,   //authoring plug-in class to add to the plug-in container
	render_in_place_fx		   //corresponding Sound Engine plug-in class
);
