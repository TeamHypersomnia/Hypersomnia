#include "augs/templates/history.hpp"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/builder/gui/builder_hierarchy_gui.h"

void builder_hierarchy_gui::perform(const builder_hierarchy_input in) {
	using namespace augs::imgui;

	(void)in;
	
	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	text("Hello world");
}
