#include "augs/templates/history.hpp"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/editor/gui/editor_layers_gui.h"

void editor_layers_gui::perform(const editor_layers_input in) {
	using namespace augs::imgui;

	(void)in;
	
	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	text("Hello world");
}
